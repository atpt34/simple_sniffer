#include "common.h"

typedef struct ipaddr_node {
    struct sockaddr_in* ipaddr;
    int total_packets;
} ipaddr_node;

typedef struct ips_structure {
    void * actual_ds;
} ips_structure;

ips_structure* ips_structure_create();
ipaddr_node* ips_structure_query(ips_structure*, ipaddr_node*);
void ips_structure_insert(ips_structure *, ipaddr_node *);
void ips_structure_print(ips_structure *);
void ips_structure_print_buf(ips_structure*, char *);
void ips_structure_destroy(ips_structure *);
void ips_structure_persist(ips_structure*, FILE *);

ipaddr_node * ipaddr_create(uint32_t);
void ipaddr_increment_total_packets(ipaddr_node *);
void ipaddr_set_total_packets(ipaddr_node *, int);
void ipaddr_print(void* );
void ipaddr_print_buf(void* , char*);
