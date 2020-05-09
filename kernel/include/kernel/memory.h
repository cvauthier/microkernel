#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/multiboot.h>

#define MAX_PAGES (4<<20)

int memory;
int nb_pages;

#ifdef _ARCH_i386

#define PAGE_SIZE 4096
#define PD_SIZE 4096
#define PT_SIZE 4096
#define NB_PDE 1024
#define NB_PTE 1024
#define FIRST_KERNEL_PDE 768

typedef uint32_t paddr_t; // Adresse physique
typedef uint32_t vaddr_t; // Adresse virtuelle
typedef uint32_t stackint_t; // Element de la pile

typedef uint32_t pde_t; // Page Directory Entry
typedef uint32_t pte_t; // Page Table Entry

#endif

void memory_setup(multiboot_info_t* mbd);

// Tas

vaddr_t get_heap_begin();

// Gestion du MMU

void load_pd(paddr_t pd_addr);
void tlb_flush();

#ifdef _ARCH_i386
#include <arch/i386/paging_tools.h>
#endif

inline pde_t *cur_pd_addr();
inline pte_t *cur_pt_addr(vaddr_t vaddr);

inline pde_t *pde_of_addr(pde_t *pd_first, vaddr_t vaddr);
inline pte_t *pte_of_addr(pte_t *pt_first, vaddr_t vaddr);

inline void pte_mkread(pte_t *e);
inline void pte_mkwrite(pte_t *e);
inline void pte_mkexec(pte_t *e);
inline void pte_noread(pte_t *e);
inline void pte_nowrite(pte_t *e);
inline void pte_noexec(pte_t *e);

inline int pte_can_read(pte_t *e);
inline int pte_can_write(pte_t *e);
inline int pte_can_exec(pte_t *e);

inline paddr_t pte_addr(pte_t *e);
inline void pte_set_addr(pte_t *e, paddr_t addr);
inline int pte_present(pte_t *e);

inline paddr_t pde_addr(pde_t *e);
inline void pde_set_addr(pde_t *e, paddr_t addr);
inline int pde_present(pde_t *e);

inline paddr_t get_paddr(vaddr_t vaddr);

inline void pd_rec_map(pde_t *pd_first, paddr_t pd_addr);

pde_t *add_pde(pde_t *pd_first, vaddr_t vaddr, paddr_t paddr); // Par défaut avec toutes les autorisations 
pte_t *add_pte(pte_t *pt_first, vaddr_t vaddr, paddr_t paddr); // Par défaut sans autorisation

void add_page(vaddr_t vaddr, paddr_t paddr);
void map_page(void *page_addr);

// Gestion des pages physiques 

paddr_t alloc_physical_page();
void free_physical_page(paddr_t page);

// Manipulation temporaire de pages
// Utilisé pour accéder temporairement à des pages physiques non mappées

vaddr_t temp_map(paddr_t paddr, int i); // 0 <= i <= 5

#endif
