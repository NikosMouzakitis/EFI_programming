#include <linux/init.h>
#include <linux/module.h>
#include <linux/efi.h>
#include <linux/rtc.h>

#define EFI_FPMURPHY_GUID EFI_GUID(0x11111111, 0x2222, 0x3333, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb)
#define EFI_FPMURPHY_GUID2 EFI_GUID(0x12121212, 0x2222, 0x3333, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb)
#define SMM_COMMUNICATION_PROTOCOL_GUID EFI_GUID(0xc68ed8e2, 0x9dc6, 0x4cbd, 0x9d, 0x94, 0xdb, 0x65, 0xac, 0xc5, 0xc3, 0x32) 

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mouzakitis Nikolaos");

/// part of the function.. copied from /drivers/rtc/rtc-efi.c
//	used to test correct get_time invocation.
static bool
convert_from_efi_time(efi_time_t *eft, struct rtc_time *wtime)
{
	memset(wtime, 0, sizeof(*wtime));

	if (eft->second >= 60)
		return false;
	wtime->tm_sec  = eft->second;

	if (eft->minute >= 60)
		return false;
	wtime->tm_min  = eft->minute;

	if (eft->hour >= 24)
		return false;
	wtime->tm_hour = eft->hour;

	if (!eft->day || eft->day > 31)
		return false;
	wtime->tm_mday = eft->day;

	if (!eft->month || eft->month > 12)
		return false;
	wtime->tm_mon  = eft->month - 1;

	if (eft->year < 1900 || eft->year > 9999)
		return false;
	wtime->tm_year = eft->year - 1900;
	return true;
}

static int efi_com_init(void)
{

	efi_time_t eft;
	efi_time_cap_t cap;
	efi_status_t status;
	struct rtc_time realt; //from rtc.h
	efi_guid_t guid = EFI_FPMURPHY_GUID;
	efi_char16_t name[] = L"TestVar";
	// to be tested with function pointer call.
	unsigned long data_size = 8;
	uint32_t attr;
	uint8_t data[8];
	int i;

	status = efi.get_time(&eft, &cap);

	if(status == EFI_SUCCESS) {
		pr_info("Success execution of efi.get_time()\n");

		if(!convert_from_efi_time(&eft, &realt)) {
			pr_info("Error on convert_from_efi_time()\n");
			return (-1);
		}

		pr_info("%s: Loaded\n",__func__);
		pr_info("efi.get_time() results\n");
		pr_info("Year: %d  Month: %d Hour: %d Min: %d Sec: %d\n", realt.tm_year, realt.tm_mon, realt.tm_hour, realt.tm_min, realt.tm_sec);

		//system reset-tested.
	//	efi.reset_system(0 , EFI_SUCCESS, 0, NULL);

		//validate efi runtime services enabled.
		if (!efi_enabled(EFI_RUNTIME_SERVICES)) {
			pr_info("%s : EFI runtime services are not enabled\n", __func__);
			return 0;
		} else {
			pr_info("%s : EFI runtime services are enabled\n", __func__);
		}

		/********************* get_variable() test. *******************************************/
		pr_info("%s :Trying to get the new created UEFI variable.\n", __func__);

		status = efi.get_variable(name, &guid, &attr, &data_size, data);

		if(status == EFI_SUCCESS) {
			pr_info("%s :Success get_variable() call\n", __func__);
			pr_info("%s :Attr returned from get_variable() call for UEFI VARIABLE is %x\n", __func__, attr);
			pr_info("\n");

		} else {
			pr_info("get_variable() failed: status: %x\n", (unsigned int)status);
			return (0);
		}
		/*****************  set_variable() test  ********************************************/


		pr_info("%s :Trying to set the new created UEFI variable.\n", __func__);
		// alter the data buffer.

		for(i = 0; i < data_size; i++)
			data[i] = 84-i; //random choosen value.
		
		status = efi.set_variable(name, &guid, attr, data_size, data);

		if(status == EFI_SUCCESS) {
			pr_info("%s :Success set_variable() call\n", __func__);
			pr_info("\n");
		} else {
			pr_info("set_variable() failed: status: %x\n", (unsigned int)status);
			return (0);
		}

		return 0;

	} else {
		pr_info("Error on efi.get_time()\n");
		return (-1);
	}

}

static void efi_com_exit(void)
{
	pr_info("%s : unloaded\n",__func__);
}

module_init(efi_com_init);
module_exit(efi_com_exit);
