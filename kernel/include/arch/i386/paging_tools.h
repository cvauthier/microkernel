#ifndef PAGING_TOOLS_H
#define PAGING_TOOLS_H

static inline pde_t *cur_pd_addr()
{
	return (pde_t*) 0xFFFFF000;
}

static inline pte_t *cur_pt_addr(vaddr_t vaddr)
{
	return (pte_t*) (0xFFC00000 | ((vaddr>>22)<<12));
}

static inline pde_t *pde_of_addr(pde_t *pd_first, vaddr_t vaddr)
{
	return pd_first + (vaddr>>22);
}

static inline pte_t *pte_of_addr(pte_t *pt_first, vaddr_t vaddr)
{
	return pt_first + ((vaddr >> 12) & 0x3FF);
}

static inline void pte_mkread(pte_t *e)
{
	*e |= 0x4; // 1 -> bit User/Supervisor
}

static inline void pte_mkwrite(pte_t *e)
{
	*e |= 0x2; // 1 -> bit Read/Write
}

static inline void pte_mkexec(pte_t *e)
{
	*e |= 0x4; // 1 -> bit User/Supervisor
}

static inline void pte_noread(pte_t *e)
{
	*e &= 0xFFFFFFFB; // 0 -> bit User/Supervisor
}

static inline void pte_nowrite(pte_t *e)
{
	*e &= 0xFFFFFFFD; // 0 -> bit Read/Write
}

static inline void pte_noexec(pte_t *e)
{
	*e &= 0xFFFFFFFB; // 0 -> bit User/Supervisor
}

static inline int pte_can_read(pte_t *e)
{
	return *e & 0x4;
}

static inline int pte_can_write(pte_t *e)
{
	return *e & 0x2;
}

static inline int pte_can_exec(pte_t *e)
{
	return *e & 0x4;
}

static inline paddr_t pte_addr(pte_t *e)
{
	return *e & 0xFFFFF000;
}

static inline void pte_set_addr(pte_t *e, paddr_t addr)
{
	*e = (*e & 0xFFF) | (addr & 0xFFFFF000);
}

static inline int pte_is_present(pte_t *e)
{
	return *e & 0x1;
}

static inline void pte_set_present(pte_t *e, int b)
{
	*e = (*e & 0xFFFFFFFE) | ((b != 0) ? 1 : 0);
}

static inline paddr_t pde_addr(pde_t *e)
{
	return *e & 0xFFFFF000;
}

static inline void pde_set_addr(pde_t *e, paddr_t addr)
{
	*e = (*e & 0xFFF) | (addr & 0xFFFFF000);
}

static inline int pde_is_present(pde_t *e)
{
	return *e & 0x1;
}

static inline void pde_set_present(pte_t *e, int b)
{
	*e = (*e & 0xFFFFFFFE) | ((b != 0) ? 1 : 0);
}

static inline paddr_t get_paddr(vaddr_t vaddr)
{
	pte_t *page_table_entry = pte_of_addr(cur_pt_addr(vaddr), vaddr);
	return (pte_addr(page_table_entry)) | (vaddr & 0xFFF);
}

static inline void pd_rec_map(pde_t *pd_first, paddr_t pd_addr)
{
	pd_first += 1023;
	*pd_first = (pd_addr & 0xFFFFF000) | 0x3; // Present + ReadWrite
}

#endif
