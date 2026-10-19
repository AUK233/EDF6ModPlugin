#pragma once

typedef struct XGS_StateMachineTimer_t {
	int pad00;
	int m_time;
} *P_XGS_StateMachineTimer;
#if 1
static_assert(offsetof(XGS_StateMachineTimer_t, m_time) == 4);
#endif

typedef struct XGS_StateMachineTable_t {
	void* vf_table;
	P_XGS_StateMachineTimer pTimer;
	char pad10[0x20];
} *P_XGS_StateMachineTable;
#if 1
static_assert(sizeof(XGS_StateMachineTable_t) == 0x30);
#endif

typedef struct XGS_LinkedListData_t {
	XGS_LinkedListData_t* pNext;
	XGS_LinkedListData_t* pBefore;
	void* pObject;
} *P_XGS_LinkedListData;
#if 1
static_assert(offsetof(XGS_LinkedListData_t, pObject) == 0x10);
#endif

typedef struct XGS_LinkedList_t {
	char pad00[0x10];
	P_XGS_LinkedListData pData;
	size_t m_count;
} *P_XGS_LinkedList;
#if 1
static_assert(offsetof(XGS_LinkedList_t, m_count) == 0x18);
static_assert(sizeof(XGS_LinkedList_t) == 0x20);
#endif
