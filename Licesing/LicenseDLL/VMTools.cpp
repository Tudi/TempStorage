#include "VMTools.h"
#include <intrin.h>
#include <memory.h>

#ifdef X64
int swallow_redpill() {
	unsigned char m[2 + 4], rpill[] = "\x0f\x01\x0d\x00\x00\x00\x00\xc3";
	*((unsigned*)&rpill[3]) = (unsigned)m;
	((void(*)())&rpill)();
	return (m[5]>0xd0) ? 1 : 0;
}

int cpuid_check2()
{
	unsigned int cpuInfo[4];
	__cpuid((int*)cpuInfo, 1);
	if( ((cpuInfo[2] >> 31) & 1) == 1)
	{
		__cpuid((int*)cpuInfo, 0x40000000);
		char hyper_vendor_id[13];
		memcpy(hyper_vendor_id + 0, &cpuInfo[1], 4);
		memcpy(hyper_vendor_id + 4, &cpuInfo[2], 4);
		memcpy(hyper_vendor_id + 8, &cpuInfo[3], 4);
		hyper_vendor_id[12] = '\0';
		if (!memcmp(hyper_vendor_id, "VMwareVMware", sizeof("VMwareVMware")))
			return 1;               // Success - running under VMware
	}
	return 0;
}

extern "C" int IsInsideVM_asm();
extern "C" void idtCheck_asm(void *);
extern "C" void sgdtCheck_asm(void *);
extern "C" void sldtCheck_asm(void *);

int IsInsideVMWare_c()
{
	int returnValue;
	__try
	{
		returnValue = IsInsideVM_asm();
	}
	__except (1)
	{
		returnValue = 0;
	}
	return returnValue;
}

int idtCheck_c()
{
	int returnValue = 0;
	__try
	{
		unsigned char idt_addr[10];
		idtCheck_asm(idt_addr);
		returnValue = idt_addr[1] > 0xd0; 
	}
	__except (1)
	{
		returnValue = 0;
	}
	return returnValue;
}

int ldtCheck_c()
{
	int returnValue = 0;
	__try
	{
		unsigned char idt_addr[20];
		sldtCheck_asm(idt_addr);
		returnValue = (idt_addr[0] != 0x00 && idt_addr[1] != 0x00) ? 1 : 0; 
	}
	__except (1)
	{
		returnValue = 0;
	}
	return returnValue;
}


int sgdtCheck_c()
{
	int returnValue = 0;
	__try
	{
		unsigned char idt_addr[10];
		sgdtCheck_asm(idt_addr);
		returnValue = idt_addr[1] > 0xd0; 
	}
	__except (1)
	{
		returnValue = 0;
	}
	return returnValue;
}

int Detect_VMware()
{
	if (cpuid_check2())
	{
		printf("CPUID said we are VMWare");
		return 1;
	}
	else if (idtCheck_c())
	{
		printf("IDT said we are a VM");
		return 1;
	}
	else if (sgdtCheck_c())
	{
		printf("SGDT said we are a VM");
		return 1;
	}
	else if (ldtCheck_c())
	{
		printf("LDT said we are a VM");
		return 1;
	}
	//	else if (IsInsideVMWare_c())
//		return 1;
//	else if (swallow_redpill())
//		return 1;               
	return 0;
}

int TestHypervisorBit()
{
	unsigned int cpuInfo[4];
	__cpuid((int*)cpuInfo, 1);
	if (((cpuInfo[2] >> 31) & 1) == 1)	//hypervisor bit
		return 1;
	return 0;
}

int Detect_VM()
{
	if (TestHypervisorBit())
	{
//		printf("Hypervisor said we are a VM");
		return 1;
	}
	else if (idtCheck_c())
	{
//		printf("IDT said we are a VM");
		return 1;
	}
	else if (sgdtCheck_c())
	{
//		printf("SGDT said we are a VM");
		return 1;
	}
	else if (ldtCheck_c())
	{
//		printf("LDT said we are a VM");
		return 1;
	}
	return 0;
}
#else
int TestHypervisorBit()
{
	unsigned int cpuInfo[4];
	__cpuid((int*)cpuInfo, 1);
	if (((cpuInfo[2] >> 31) & 1) == 1)	//hypervisor bit
		return 1;
	return 0;
}

int Detect_VM()
{
	if (TestHypervisorBit())
	{
		printf("Hypervisor said we are a VM");
		return 1;
	}
	return 0;
}
#endif

#ifdef USE_VMWARE_SUGGESTED
int cpuid_check()
{
	unsigned int eax, ebx, ecx, edx;
	char hyper_vendor_id[13];

	cpuid(0x1, &eax, &ebx, &ecx, &edx);
	if (bit 31 of ecx is set) {
		cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
		memcpy(hyper_vendor_id + 0, &ebx, 4);
		memcpy(hyper_vendor_id + 4, &ecx, 4);
		memcpy(hyper_vendor_id + 8, &edx, 4);
		hyper_vendor_id[12] = '\0';
		if (!strcmp(hyper_vendor_id, "VMwareVMware"))
			return 1;               // Success - running under VMware
	}
	return 0;
}

int dmi_check(void)
{
	char string[10];
	GET_BIOS_SERIAL(string);

	if (!memcmp(string, "VMware-", 7) || !memcmp(string, "VMW", 3))
		return 1;                       // DMI contains VMware specific string.
	else
		return 0;
}

#define VMWARE_HYPERVISOR_MAGIC 0x564D5868
#define VMWARE_HYPERVISOR_PORT  0x5658

#define VMWARE_PORT_CMD_GETVERSION      10

#define VMWARE_PORT(cmd, eax, ebx, ecx, edx)                            \
        __asm__("inl (%%dx)" :                                          \
                        "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx) :    \
                        "0"(VMWARE_HYPERVISOR_MAGIC),                   \
                        "1"(VMWARE_PORT_CMD_##cmd),                     \
                        "2"(VMWARE_HYPERVISOR_PORT), "3"(UINT_MAX) :    \
                        "memory");

int hypervisor_port_check(void)
{
	unsigned int eax, ebx, ecx, edx;
	VMWARE_PORT(GETVERSION, eax, ebx, ecx, edx);
	if (ebx == VMWARE_HYPERVISOR_MAGIC)
		return 1;               // Success - running under VMware
	else
		return 0;
}

int Detect_VMware(void)
{
	if (cpuid_check())
		return 1;               // Success running under VMware.
	else if (dmi_check() && hypervisor_port_check())
		return 1;
	else if (swallow_redpill() == 1)
		return 1;
	return 0;
}
#endif


#ifndef X64
/* ScoopyNG - The VMware detection tool
* Version v1.0
*
* Tobias Klein, 2008
* www.trapkit.de
*/

#include <windows.h>
#include <excpt.h>
#include <stdio.h>

#define DEBUG	0
#define EndUserModeAddress (*(UINT_PTR*)0x7FFE02B4)

typedef LONG(NTAPI *NTSETLDTENTRIES)(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);

unsigned long
get_idt_base(void)
{
	unsigned char	idtr[6];
	unsigned long	idt = 0;

	_asm sidt idtr
	idt = *((unsigned long *)&idtr[2]);

	return (idt);
}

unsigned long
get_ldtr_base(void)
{
	unsigned char   ldtr[5] = "\xef\xbe\xad\xde";
	unsigned long   ldt = 0;

	_asm sldt ldtr
	ldt = *((unsigned long *)&ldtr[0]);

	return (ldt);
}

unsigned long
get_gdt_base(void)
{
	unsigned char   gdtr[6];
	unsigned long   gdt = 0;

	_asm sgdt gdtr
	gdt = *((unsigned long *)&gdtr[2]);

	return (gdt);
}

void
test1(void)
{
	unsigned int 	idt_base = 0;

	idt_base = get_idt_base();

	printf("[+] Test 1: IDT\n");
	printf("IDT base: 0x%x\n", idt_base);

	if ((idt_base >> 24) == 0xff) {
		printf("Result  : VMware detected\n\n");
		return;
	}

	else {
		printf("Result  : Native OS\n\n");
		return;
	}
}

void
test2(void)
{
	unsigned int	ldt_base = 0;

	ldt_base = get_ldtr_base();

	printf("\n[+] Test 2: LDT\n");
	printf("LDT base: 0x%x\n", ldt_base);

	if (ldt_base == 0xdead0000) {
		printf("Result  : Native OS\n\n");
		return;
	}

	else {
		printf("Result  : VMware detected\n\n");
		return;
	}
}

void
test3(void)
{
	unsigned int	gdt_base = 0;

	gdt_base = get_gdt_base();

	printf("\n[+] Test 3: GDT\n");
	printf("GDT base: 0x%x\n", gdt_base);

	if ((gdt_base >> 24) == 0xff) {
		printf("Result  : VMware detected\n\n");
		return;
	}

	else {
		printf("Result  : Native OS\n\n");
		return;
	}
}

// Alfredo Andrés Omella's (S21sec) STR technique
void
test4(void)
{
	unsigned char	mem[4] = { 0, 0, 0, 0 };

	__asm str mem;

	printf("\n[+] Test 4: STR\n");
	printf("STR base: 0x%02x%02x%02x%02x\n", mem[0], mem[1], mem[2], mem[3]);

	if ((mem[0] == 0x00) && (mem[1] == 0x40))
		printf("Result  : VMware detected\n\n");
	else
		printf("Result  : Native OS\n\n");
}

void
test5(void)
{
	unsigned int	a, b;

	__try {
		__asm {

			// save register values on the stack
			push eax
				push ebx
				push ecx
				push edx

				// perform fingerprint
				mov eax, 'VMXh'	// VMware magic value (0x564D5868)
				mov ecx, 0Ah		// special version cmd (0x0a)
				mov dx, 'VX'		// special VMware I/O port (0x5658)

				in eax, dx			// special I/O cmd

				mov a, ebx			// data 
				mov b, ecx			// data	(eax gets also modified but will not be evaluated)

				// restore register values from the stack
				pop edx
				pop ecx
				pop ebx
				pop eax
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

#if DEBUG == 1
	printf("\n [ a=%x ; b=%d ]\n\n", a, b);
#endif

	printf("\n[+] Test 5: VMware \"get version\" command\n");

	if (a == 'VMXh') {		// is the value equal to the VMware magic value?
		printf("Result  : VMware detected\nVersion : ");
		if (b == 1)
			printf("Express\n\n");
		else if (b == 2)
			printf("ESX\n\n");
		else if (b == 3)
			printf("GSX\n\n");
		else if (b == 4)
			printf("Workstation\n\n");
		else
			printf("unknown version\n\n");
	}
	else
		printf("Result  : Native OS\n\n");
}

void
test6(void)
{
	unsigned int	a = 0;

	__try {
		__asm {

			// save register values on the stack
			push eax
				push ebx
				push ecx
				push edx

				// perform fingerprint
				mov eax, 'VMXh'		// VMware magic value (0x564D5868)
				mov ecx, 14h		// get memory size command (0x14)
				mov dx, 'VX'		// special VMware I/O port (0x5658)

				in eax, dx			// special I/O cmd

				mov a, eax			// data 

				// restore register values from the stack
				pop edx
				pop ecx
				pop ebx
				pop eax
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	printf("\n[+] Test 6: VMware \"get memory size\" command\n");

	if (a > 0)
		printf("Result  : VMware detected\n\n");
	else
		printf("Result  : Native OS\n\n");
}

int
test7_detect(LPEXCEPTION_POINTERS lpep)
{
	printf("\n[+] Test 7: VMware emulation mode\n");

	if ((UINT_PTR)(lpep->ExceptionRecord->ExceptionAddress) > EndUserModeAddress)
		printf("Result  : VMware detected (emulation mode detected)\n\n");
	else
		printf("Result  : Native OS or VMware without emulation mode\n"
		"          (enabled acceleration)\n\n");

	return (EXCEPTION_EXECUTE_HANDLER);
}

void __declspec(naked)
test7_switchcs()
{
	__asm {
		pop eax
			push 0x000F
			push eax
			retf
	}
}

// Derek Soeder's (eEye Digital Security) VMware emulation test
void
test7(void)
{
	NTSETLDTENTRIES ZwSetLdtEntries;
	LDT_ENTRY csdesc;

	ZwSetLdtEntries = (NTSETLDTENTRIES)GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwSetLdtEntries");

	memset(&csdesc, 0, sizeof(csdesc));

	csdesc.LimitLow = (WORD)(EndUserModeAddress >> 12);
	csdesc.HighWord.Bytes.Flags1 = 0xFA;
	csdesc.HighWord.Bytes.Flags2 = 0xC0 | ((EndUserModeAddress >> 28) & 0x0F);

	ZwSetLdtEntries(0x000F, ((DWORD*)&csdesc)[0], ((DWORD*)&csdesc)[1], 0, 0, 0);

	__try {
		test7_switchcs();
		__asm {
			or eax, -1
				jmp eax
		}
	}
	__except (test7_detect(GetExceptionInformation())) { }
}

int
main(void)
{
	printf("\n\n####################################################\n");
	printf("::       ScoopyNG - The VMware Detection Tool     ::\n");
	printf("::              Windows version v1.0              ::\n\n");

	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();

	printf("::                   tk,  2008                    ::\n");
	printf("::               [ www.trapkit.de ]               ::\n");
	printf("####################################################\n\n");

	return 0;
}
#endif