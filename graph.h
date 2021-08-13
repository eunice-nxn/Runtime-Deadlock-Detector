typedef struct Node {
	long int thread_id;
	pthread_mutex_t * mutex;
	struct Node * next;
} Node;

typedef struct Edge {
	struct Node * u;
	struct Node * v;
	struct Edge * next;
	int visited;
} Edge;

typedef struct Graph {
	int n_list_size;
	int e_list_size;
	struct Node * n_list_first; // all Nodes
	struct Edge * e_list_first; // all edges
} Graph;

Node * node_init(long int thread_id, pthread_mutex_t * mutex);

Edge * edge_init(Node * u, Node * v);

Graph * graph_init ();

int print_graph (Graph * g);

int delete_edge (Graph * g, Edge * e);

int delete_node (Graph * g, Node * n);

int insert_edge (Graph * g, Edge * e);

int insert_node (Graph * g, Node * n);

