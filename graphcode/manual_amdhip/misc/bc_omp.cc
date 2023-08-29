#include"bc_dsl_new.h"

#include <iostream>

#define NL "\n"

using namespace std;

void Compute_BC_new(graph& g,double* BC,std::set<int>& sourceSet)
{
  // omp_set_num_threads(1);
  double* sigma=new double[g.num_nodes()];
  double* delta=new double[g.num_nodes()];
  #pragma omp parallel for
  for (int t = 0; t < g.num_nodes(); t ++) 
  {
    BC[t] = 0;
  }
  std::set<int>::iterator itr;
  for(itr=sourceSet.begin();itr!=sourceSet.end();itr++)
  {
    int src = *itr;
    int* bfsDist=new int[g.num_nodes()];
    #pragma omp parallel for
    for (int t = 0; t < g.num_nodes(); t ++) 
    {
      delta[t] = 0;
      bfsDist[t] = -1;
      sigma[t] = 0;
    }
    sigma[src] = 1;
    bfsDist[src] = 0;
    std::vector<std::vector<int>> levelNodes(g.num_nodes()) ;
    std::vector<std::vector<int>>  levelNodes_later(omp_get_max_threads()) ;
    std::vector<int>  levelCount(g.num_nodes()) ;
    int phase = 0 ;
    levelNodes[phase].push_back(src) ;
    int bfsCount = 1 ;
    levelCount[phase] = bfsCount;

    cout << "Current src: " << *itr << NL;

    while ( bfsCount > 0 )
    {
       int prev_count = bfsCount ;
      bfsCount = 0 ;
      #pragma omp parallel for
      for( int l = 0; l < prev_count ; l++)
      {
        int v = levelNodes[phase][l] ;
        for(int edge = g.indexofNodes[v] ; edge < g.indexofNodes[v+1] ; edge++) {
          int nbr = g.edgeList[edge] ;
          int dnbr ;
          if(bfsDist[nbr]<0)
          {
            dnbr = __sync_val_compare_and_swap(&bfsDist[nbr],-1,bfsDist[v]+1);
            if (dnbr < 0)
            {
              int num_thread = omp_get_thread_num();
               levelNodes_later[num_thread].push_back(nbr) ;
            }
          }
        }
        for (int edge = g.indexofNodes[v]; edge < g.indexofNodes[v+1]; edge ++) 
        {int w = g.edgeList[edge] ;
          if(bfsDist[w]==bfsDist[v]+1)
          {
            #pragma omp atomic
            sigma[w] = sigma[w] + sigma[v];
          }
        }
      }
      phase = phase + 1 ;
      for(int i = 0;i < omp_get_max_threads();i++)
      {
         levelNodes[phase].insert(levelNodes[phase].end(),levelNodes_later[i].begin(),levelNodes_later[i].end());
         bfsCount=bfsCount+levelNodes_later[i].size();
         levelNodes_later[i].clear();
      }
       levelCount[phase] = bfsCount ;
    }
    phase = phase -1 ;

    while (phase > 0)
    {
      #pragma omp parallel for
      for( int l = 0; l < levelCount[phase] ; l++)
      {
        int v = levelNodes[phase][l] ;
        for (int edge = g.indexofNodes[v]; edge < g.indexofNodes[v+1]; edge ++) 
        {
			int w = g.edgeList[edge] ;
			if(bfsDist[w]==bfsDist[v]+1)
				delta[v] = delta[v] + (sigma[v] / sigma[w]) * (1 + delta[w]);
        }
        BC[v] = BC[v] + delta[v];
      }
      phase = phase - 1 ;
    }
  }
}

int main(int argc , char ** argv)
{
  graph G(argv[1]);
  G.parseGraph();
  bool printAns = false;

  std::set<int> src;

  if(argc>3) { // ./a.out inputfile srcFile -p
      printAns = true;
  }


  std::string line;
  std::ifstream srcfile(argv[2]);

  int nodeVal;
  while ( std::getline (srcfile,line) ) {
   std::stringstream ss(line);
   ss>> nodeVal;
   src.insert(nodeVal);
  }

  srcfile.close();
  printf("#srces:%d\n",src.size());
  //==========================================




    //~ cudaEvent_t start, stop; // should not be here!
    //~ cudaEventCreate(&start);
    //~ cudaEventCreate(&stop);
    //~ float milliseconds = 0;
    //~ cudaEventRecord(start,0);
    unsigned V = G.num_nodes();
    unsigned E = G.num_nodes();
    double* BC = (double *)malloc(sizeof(double)*V);
    Compute_BC_new(G,BC,src);

    // int LIMIT = 9;
    // if(printAns)
    //  LIMIT=V;

    for(auto itr=src.begin();itr!=src.end();itr++) {
      printf("%d %lf\n", *itr, BC[*itr]);
    }

    //~ cudaDeviceSynchronize();

    //~ cudaEventRecord(stop,0);
    //~ cudaEventSynchronize(stop);
    //~ cudaEventElapsedTime(&milliseconds, start, stop);
    //~ printf("Time taken by function to execute is: %.6f ms\n", milliseconds);
    // cudaCheckError();

  return 0;

}
