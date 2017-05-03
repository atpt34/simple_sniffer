#include "data_structure.h"
#include "red_black_tree.h"

ipaddr_node * ipaddr_create(uint32_t inaddr) {
    ipaddr_node * node = (ipaddr_node*) malloc(sizeof(ipaddr_node));
    struct sockaddr_in * sa = (struct sockaddr_in *) malloc (sizeof(struct sockaddr_in));
    (sa->sin_addr).s_addr = inaddr;
    node->ipaddr = sa;
    node->total_packets = 0;
    return node;
}

void ipaddr_increment_total_packets(ipaddr_node * node) {
    node->total_packets++;
}

void ipaddr_set_total_packets(ipaddr_node * node, int value) {
    node->total_packets = value;
}

void ipaddr_destroy(void* a) {
    free(((ipaddr_node*)a)->ipaddr);
    free((ipaddr_node*)a);
}

int ipaddr_compare(const void * a, const void * b) {
    if( ((ipaddr_node*)a)->ipaddr->sin_addr.s_addr > ((ipaddr_node*)b)->ipaddr->sin_addr.s_addr) return 1;
    if( ((ipaddr_node*)a)->ipaddr->sin_addr.s_addr < ((ipaddr_node*)b)->ipaddr->sin_addr.s_addr) return -1;
    return 0;
}

void ipaddr_print(void* a) {
    ipaddr_node* node = (ipaddr_node*) a;
    printf ("%s\t%u\n", inet_ntoa(node->ipaddr->sin_addr), node->total_packets);
}

void ipaddr_print_file(void* a, FILE* fp) {
    ipaddr_node* node = (ipaddr_node*) a;
    fprintf (fp, "%s\t%u\n", inet_ntoa(node->ipaddr->sin_addr), node->total_packets);
}

void info_print(void *a) {;}
void info_destroy(void *a) {;}

ips_structure * ips_structure_create() {
    rb_red_blk_tree* ds = RBTreeCreate(ipaddr_compare, ipaddr_destroy, info_destroy, ipaddr_print, ipaddr_print_file, info_print);
    ips_structure* ips_ds = (ips_structure *) malloc(sizeof(ips_structure));
    ips_ds->actual_ds = ds;
    return ips_ds;
}
ipaddr_node * ips_structure_query(ips_structure* ips_ds, ipaddr_node * node) {
    //return (ipaddr_node *) (RBExactQuery(ips_ds->actual_ds, node)->key);
    rb_red_blk_node* ex_node;
    if ( (ex_node = RBExactQuery((rb_red_blk_tree*)(ips_ds->actual_ds), node)) ) {
    //if ( (ex_node = RBExactQuery(ips_ds->actual_ds, node)) ) {
        return (ipaddr_node*) (ex_node->key);
    } else {
        return NULL;
    }
}
void ips_structure_insert(ips_structure * ips_ds, ipaddr_node * node) {
    RBTreeInsert((rb_red_blk_tree*)(ips_ds->actual_ds), node, 0);
}
void ips_structure_print(ips_structure * ips_ds) {
    RBTreePrint((rb_red_blk_tree*)(ips_ds->actual_ds));
}
void ips_structure_destroy(ips_structure * ips_ds) {
    RBTreeDestroy((rb_red_blk_tree*)(ips_ds->actual_ds));
    free(ips_ds);
}

void ips_structure_persist(ips_structure* ips_ds, FILE * fstore) {
    RBTreePrintFile((rb_red_blk_tree*)(ips_ds->actual_ds), fstore);
}
