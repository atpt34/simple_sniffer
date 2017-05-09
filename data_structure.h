/*
    Data structure interface to store IP address data.
 */
#include "common.h"

// Struct to hold IP address and packets counter.
typedef struct ipaddr_node {
    struct sockaddr_in* ipaddr;
    int total_packets;
} ipaddr_node;

// Struct to encapsulate acutal implementation of
// data structure that should provide O(log N) operations.
typedef struct ips_structure {
    void * actual_ds;
} ips_structure;

// Returns new ips_strucuture.
ips_structure* ips_structure_create();

// Searches for given ipaddr_node in ips_structure.
// Returns pointer to exisiting, NULL otherwise.
ipaddr_node* ips_structure_query(ips_structure*, ipaddr_node*);

// Insert new ipaddr_node to ips_structure.
void ips_structure_insert(ips_structure *, ipaddr_node *);

// Prints ips_structure to stdout.
void ips_structure_print(ips_structure *);

// Prints ips_structure to char buffer.
void ips_structure_print_buf(ips_structure*, char *);

// Clean up space occupied by ips_structure.
void ips_structure_destroy(ips_structure *);

// Prints ips_structure to given file.
void ips_structure_persist(ips_structure*, FILE *);

// Creates new ipaddr_node.
// Takes ip address as argument.
ipaddr_node * ipaddr_create(uint32_t);

// Clean up space occupied by ipaddr_node.
void ipaddr_destroy(void* );

// Increments by 1 packets counter of ipaddr_node.
void ipaddr_increment_total_packets(ipaddr_node *);

// Prints ipaddr_node to stdout.
void ipaddr_print(void* );

// Prints ipaddr_node to char buffer.
void ipaddr_print_buf(void* , char*);
