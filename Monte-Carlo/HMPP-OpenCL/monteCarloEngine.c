//monteCarloEngine.cu
//Scott Grauer-Gray
//May 10, 2012
//Function for running Monte Carlo on the GPU

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "monteCarloKernels.c"
#include "mt19937.c"


#pragma hmpp samples codelet, target=OpenCL, args[samplePrices;sampleWeights].io=out
void monteCarloKernel (int n, float* samplePrices, float* sampleWeights, float dt, mt19937state* randStates, monteCarloOptionStruct optionStruct)
{
  int sampNum;

  #pragma hmppcg parallel
  for (sampNum = 0; sampNum < n; ++sampNum)
  {
      
      float path[SEQUENCE_LENGTH];
      int i;

      #pragma hmppcg noParallel
      for (i = 0; i < SEQUENCE_LENGTH; i++)
      {
          path [i] = START_PATH_VAL;
      }

      int optionStructNum = 0;
      
      float weight = DEFAULT_SEQ_WEIGHT;
      path [0] = getProcessValX0(optionStruct);
      size_t ii;

      #pragma hmppcg noParallel
      for (ii = 1; ii < SEQUENCE_LENGTH; ii++) 
      {
          float t = ii * dt;
	  
	  float randVal = generateRandFloat(randStates, sampNum);
          float inverseCumRandVal = compInverseNormDist (randVal); 
	  path [ii] = processEvolve(t, path [ii - 1], dt, inverseCumRandVal, optionStruct); 
      }

      float price = getPrice (path [SEQUENCE_LENGTH-1]);
      
      samplePrices [sampNum] = price;
      sampleWeights [sampNum] = DEFAULT_SEQ_WEIGHT;
  }
}


//run monte carlo...
void runMonteCarlo ()
{
  //declare and initialize the struct used for the option
  monteCarloOptionStruct optionStruct;
  optionStruct.riskVal = RISK_VAL;
  optionStruct.divVal = DIV_VAL;
  optionStruct.voltVal = VOLT_VAL;
  optionStruct.underlyingVal = UNDERLYING_VAL;
  optionStruct.strikeVal = STRIKE_VAL;
  optionStruct.discountVal = DISCOUNT_VAL;

  struct timeval begin,end;
  mt19937state* randStates = (mt19937state*) malloc (NUM_SAMPLES * sizeof (mt19937state));

  // Accelerator Version
  {
    //declare pointers for data on CPU
    float* samplePrices = (float*) malloc (NUM_SAMPLES * sizeof (float));
    float* sampleWeights = (float*) malloc (NUM_SAMPLES * sizeof (float));

    int numSamps = NUM_SAMPLES;
    printf("Number of Samples: %d\n", numSamps);
    int sampNum;
    for (sampNum = 0; sampNum < numSamps; sampNum++)
    {
        initializeMersenneState (&(randStates[sampNum]), rand ());
    }

    #pragma hmpp samples allocate, args[samplePrices;sampleWeights;randStates].size={numSamps}
   
    #pragma hmpp samples advancedload, args[randStates]

    gettimeofday(&begin,NULL);

    //run Monte-Carlo on GPU using HMPP
    #pragma hmpp samples callsite
    monteCarloKernel (NUM_SAMPLES, samplePrices, sampleWeights, (1.0f / (float)SEQUENCE_LENGTH), randStates, optionStruct);

    #pragma hmpp samples release
    gettimeofday(&end,NULL);

    //retrieve the average price
    float cumPrice = 0.0f;
	  
    //add all the computed prices together
    int numSamp;
    for (numSamp = 0; numSamp < NUM_SAMPLES; numSamp++)
      {
	cumPrice += samplePrices [numSamp];
      }
	  
    float avgPrice = cumPrice / NUM_SAMPLES;
    printf("Average price on GPU: %f\n", avgPrice);
    printf("Runtime on GPU: %f ms\n", (((end.tv_sec - begin.tv_sec) * 1e6 + (end.tv_usec - begin.tv_usec)) / 1000.0f));

    free(samplePrices);
    free(sampleWeights);
  }  


  // CPU Version
  {
    //declare pointers for data on CPU
    float* samplePricesCpu;
    float* sampleWeightsCpu;
    float* timesCpu;
	  
    //allocate space for data on CPU
    samplePricesCpu  = (float*) malloc (NUM_SAMPLES * sizeof (float));
    sampleWeightsCpu = (float*) malloc (NUM_SAMPLES * sizeof (float));
    timesCpu         = (float*) malloc (NUM_SAMPLES * sizeof (float));
    
    int sampNum;
    for (sampNum = 0; sampNum < NUM_SAMPLES; sampNum++)
      {
	initializeMersenneState (&randStates [sampNum], rand ());
      }

    gettimeofday(&begin,NULL);
    monteCarloKernel (NUM_SAMPLES, samplePricesCpu, sampleWeightsCpu, (1.0f / (float)SEQUENCE_LENGTH), randStates, optionStruct);
    //monteCarloGpuKernelCpu (samplePricesCpu, sampleWeightsCpu, timesCpu, (1.0f / (float)SEQUENCE_LENGTH), optionStructs);
    gettimeofday(&end,NULL);

    //retrieve the average price
    float cumPrice = 0.0f;
	  
    //add all the computed prices together
    int numSamp;
    for (numSamp = 0; numSamp < NUM_SAMPLES; numSamp++)
      {
	cumPrice += samplePricesCpu[numSamp];
      }
	  
    float avgPrice = cumPrice / NUM_SAMPLES;
    printf("Average price on CPU: %f\n", avgPrice);
    printf("Runtime on CPU: %f ms\n", (((end.tv_sec - begin.tv_sec) * 1e6 + (end.tv_usec - begin.tv_usec)) / 1000.0f));

    //free memory space on the CPU
    free(samplePricesCpu);
    free(sampleWeightsCpu);
    free(timesCpu);
    free(randStates);
  }
}

