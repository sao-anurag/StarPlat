#include "graph_mpi_weight.hpp"
#include <assert.h>
#include <queue>


struct vertex_properties ;
struct edge_elements ;


struct edge_elements {


  // struct to store entities relevant to an edge for the max flow problem.
  int flow ; // current flow value through the edge.
  int capacity ; // capacity of the edge.
  int residual ; // To represent the residual graph.
  vertex_properties *source; // vertex type struct containing the source vertex of the edge.
  vertex_properties *destination ; // vertex type struct containing the destination vertex of the edge.

};

struct vertex_properties {

  
  // struct to store all information relevant to a vertex.
  bool active ; // whether the vertex is active or not,
  int excess ; // amount of excess flow passing through the vertex.
  int vertex_id ; // vertex_id of the vertex.
  int height ; // height of the vertex.
  edge_elements *current_edge ; // current selected edge through which flow is being passed through the vertex.

  bool operator< (const vertex_properties &other)  {
    
    // for storing the vertices in an associative container. To change to Hash type container.
    return vertex_id < other.vertex_id ;  
  }
  
  // for use in visited array. 
  int operator[] (vertex_properties a) {

    return a.vertex_id ;
  }
} ;



bool operator< (const vertex_properties &a, const vertex_properties &b) {

  // overload comparator so that vertex_properties may be used in the map.
  return a.vertex_id < b.vertex_id ;
}



class network_flow : public graph {

/*
 * network_flow type graph based on the class graph.
 * additionally stores edge capacities and current vertices.
 * Also maintains a residual height
 * Each vertex has an excess and a height associated with it.
 * */

private:

  // source and sink to be defined by user. 
  vertex_properties source ;
  vertex_properties sink ;

  // The adjacency list of the graph.
  // TO:DO - Change to CSR/ unordered_map post discussion.
  map <vertex_properties, vector<edge_elements> > adj;

public:


  void select_source (vertex_properties source) {
    
    // set source of the network_flow. 
    this->source = source ;
  }


  void select_sink (vertex_properties sink) {

    // set sink of the network flow.
    this->sink = sink ;
  }


  void initialize () {
    /*
     * initialize the adjacency list of the array and create the max flow.*/
    

    // retreive edge list from the parsed graph. It is actually sent back as an adjacency list for some reason.
    map <int, vector<edge>> adj_list = getEdges();   

    // TO:DO change find a better way to represent the adjacency list. 
    for (auto edge:adj_list) {
    
      // for every vertex in adjacency list (confusion caused because of naming conventions prior).
      edge_elements edge_elements_new;
  

      // Make the vertex properties struct for current vertex. Initialize the the members of the struct.
      vertex_properties this_vertex;
      this_vertex.vertex_id = edge.first;
      this_vertex.active = true;
      this_vertex.excess = 0;

      // Create empty vector just in case this is not supported.
      if (adj.find (this_vertex) == adj.end ()) {
        adj[this_vertex] = {};
      }

      // Fill up the adjacency list (vector) for the current vertex.
      for (auto edge_props:edge.second) {


        // create a destination vertex, initialize all its members.
        vertex_properties destination ;
        destination.excess = 0 ;
        destination.active = true ;
        destination.vertex_id = edge_props.destination ;


        // initialize the edge struct and all its members.
        edge_elements_new.flow = 0 ;
        edge_elements_new.capacity = edge_props.weight ;
        edge_elements_new.destination = &destination ;

        // Add the edge to the adjacency matrix.
        adj[this_vertex].push_back (edge_elements_new) ;
         
      }
    }
    // End To:DO

    // initialize the heights and the excesses.
    set_up_heights () ;
    set_up_excess () ;
  }

  void set_up_excess () {

    
    // set up the excesses. initialize source to be sum of all capacities outgoing and all others to 0.
    for (auto v_edge:adj[source]) {
      vertex_properties v= *v_edge.destination ;
      source.excess += v_edge.capacity ;
    }

    // all else is already at 0 ;
  }
  void set_up_heights () {

    /* 
     * Do a BFS to find the heights of all the vertices.
     * */
    queue <vertex_properties> q;
    q.push (source) ;
    vector<vertex_properties> visited (num_nodes ()) ;

    
    // Run the BFS for loop.
    while (!q.empty ()) {
      
      vertex_properties u = q.front () ;
      

      for (auto v_edge : adj[u]) {

        // v is an edge element.
        // Therefore, must retreive the destination vertex from it.
        vertex_properties v = *(v_edge.destination) ;

       // To Do : Fix the [] operator overlaod. For some reason, this is not working 
        if (!visited[v]) { 


          // Cannot index into I guess . 
          q.push (v) ;
          visited[v]=true ;
        } 
      }
    }
  }

  

  void relabel (vertex_properties relabel_this_vertex) {
    
    // relabel the passed vertex.. Change it's height to the lowest height among it's adjacent vertics increased by 1
    //
    int minim = 1000000000 ;
    for (auto edge : adj[relabel_this_vertex]) {

      minim = min (minim, edge.destination->height+1) ;
    }
    relabel_this_vertex.height = minim ;
  }



  void push (vertex_properties active_vertex) {


    // check whether the current edge is still a valid edge to push heights to.
    if (active_vertex.height != active_vertex.current_edge->destination->height+1) {

      // if not reset the current edge for the current vertex.
      reset_current_edge (active_vertex) ;
    }
    // push flow from a particular vertex to all subsequent vertices .. 
    int curr_flow = min (active_vertex.excess, active_vertex.current_edge->flow) ;

    // update all relevant values
    active_vertex.current_edge->residual += curr_flow ;
    active_vertex.current_edge->flow -= curr_flow ;      
    active_vertex.excess -= curr_flow ;

    // TO:DO 
    // When to Deactivate the Vertex ?? 
  }



  void reset_current_edge (vertex_properties active_vertex) {


    // search for edges leading to vertices with height exactly lesser by 1.
    for (auto v_edge:adj[active_vertex]) {

      // check whether v_edge satisfies the property.
      if (v_edge.destination->height == active_vertex.height+1) {

        // update the active vertex's edge.
        active_vertex.current_edge = &v_edge ;
        return ;
      }
    }

    // if control reaches here, relabelling is needed .
    relabel(active_vertex) ;
  }
};
































  
  
  
  
