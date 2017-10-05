#pragma once

/*
Store / Save/ Load PC related info
Could have been done with STD vector. But maybe in the future we want to provide an API to COM objects or C# .... Using basic data types ensures cross platform / language compatibility
*/

#define CURRENT_PACKER_VERSION		0
#define ALLOC_BUFFER_EXTRA_SIZE		64000	// over allocate to not spam data alloc and copy
#define MAX_EXPECTED_FILE_SIZE		(100*1024*1024)

enum DataCollectionEncryptionTypes
{
	DCE_NOT_INITIALIZED			= 0,
	DCE_INTERNAL_CyclicXOR_KEY	= 1,
	DCE_EXTERNAL_FINGERPRINT	= 2,
	DCE_NONE					= 3,
	DCE_INVALID_ENCRYPTION_TYPE
};

/// @cond DEV
#pragma pack(push,1)
//data that is platform invariant and serializable
struct DataBlockHeader
{
	unsigned char	Type;		// type of the data block
	unsigned int	Size;		// size of the data block
	unsigned int	CRC;		// extra feature to make sure data integrity remains. In memory tempering chance should be smaller
	unsigned char	Data[1];	// fake, we will use dynamic size
};
// header for a list of data blocks
struct DataCollectionHeader
{
	unsigned char	Ver;		// just in case we mess up and we need to use multiple versions. Hope we will never use this field
	unsigned short	Count;		// number of data blocks in this collection
	unsigned int	Size;		// sum of datablock sizes
	unsigned char	EncryptType;// 
	unsigned int	XORKey;		// very basic obfuscation to avoid human readable format. Keep this in the header in case for security reasons we never unpack it, we can pass it from buffer to buffer.
	unsigned char	Blocks[1];	// should be dynamic
}; 
#pragma pack(pop)

#define IsDBFingerprintEssential(type) (type==DB_MAC_ADDRESS || type==DB_CPU_ID || type==DB_UUID || type==DB_UUID_REMOTE)

enum DataBlockTypes
{
	DB_INVALID_UNINITIALIZED = 0,
	DB_MAC_ADDRESS,					// this is one of the multiple possible MAC addresses
	DB_CPU_ID,						// The CPU ID returned by the system. There could be multiple CPUs in a system. It depends on the OS API which one we will store
	DB_UUID,						// this is the BIOS ID. THere is a chance this will be unique even on VMs
	DB_UUID_REMOTE,					// this is the BIOS ID obtained from a remote fingerprinting service
	DB_MB_SN,						// Mother board serial number. Not used at the moment
	DB_C_SN,						// Serial number of the C drive, this is available only for windows
	DB_COMPUTER_NAME,				// Computer name should be unique in a workgroup or a domain
	DB_NETBIOS_NAME,				// old workgroup name
	DB_DOMAIN_NAME,					// domain we are part of
	DB_MACHINE_ROLE,				// what kind of PC is this for the client ? Server ? Client? Workstation ... ?
	DB_CLIENT_NAME,					// custom string entered by the client
	DB_USELESS_STUFFING_MIN,		// does not have any meaning. In case you wish to inspect it than waste your time with it
	DB_USELESS_STUFFING2,			// does not have any meaning. In case you wish to inspect it than waste your time with it
	DB_USELESS_STUFFING3,			// does not have any meaning. In case you wish to inspect it than waste your time with it
	DB_USELESS_STUFFING4,			// does not have any meaning. In case you wish to inspect it than waste your time with it
	DB_USELESS_STUFFING_MAX,		// does not have any meaning. In case you wish to inspect it than waste your time with it
	DB_DATE_STAMP,
	DB_XOR_KEY,
	DB_COMPRESSED_LIST,				// should generate a new DB_Block that should get parsed recursively..
	DB_LICENSE_NODE,				// some project+feature+activationkey structure
	DB_LICENSE_START,
	DB_LICENSE_DURATION,
	DB_LICENSE_END,
	DB_LICENCE_CONTACT,				// who to contact to renegotiate the contract for this license
	DB_RAW_FULL_LIST_CONTENT,		// special case when we load up content as 1 memory block
	DB_GRACE_PERIOD,				// even after license expires we will provide a grace period functionality
	DB_LIST_FINGERPRINT,			// mark that this is a valid list, this is used to guess if decoding went well
	DB_MAX_TYPES	// non used value. Used for safety checks
};

enum DataAllocReturnCodes
{
	DALLOC_SUCCESS = 0,
	DALLOC_FAIL = 1,
};

enum DataAddReturnCodes
{
	DADD_SUCCESS = 0,
	DADD_FAIL = 1,
};

enum DP_ErrorCodes
{
	DP_DATA_INTEGRITY_FAILURE = -1,
};

//! not thread safe
class GenericDataStore
{
public:
	GenericDataStore();
	~GenericDataStore();
	int						PushData(const void *buff, int Size, int Type);	// add a new data block to our list
	DataCollectionHeader	*GetData(){ return Data; };					// return the buffer used to store the list
	int						GetDataSize(){ if (Data) return (int)Data->Size; else return 0; }			// number of bytes the store takes
	int						SaveToFile(const char *FileName);			// Obfuscate + save the content of the list
	int						LoadFromFile(const char *FileName);			// load + Deobfuscate the content of a list
	int						SetEncription(unsigned char EncryptType);	// do we use internal encryption or external ? Maybe in the future more modes will be added
	int						IsDataValid();								// try to guess if the content loaded into the store is a valid store
	int						GetDataDup(char **Data, int *Size, int Type);//duplicate a field content into a newly allocated buffer. If it exists
	int						MergeLicenseNodesFrom(GenericDataStore * gds);// move fields from src store to this store
	int						HasNode(const void *buff, int Size, int Type);
	int						GetNodeCount();								// Count number of nodes. Used for debugging

	void					PrintContent();								// Debugging the content of the list
	void					DisposeData();								// get rid of the content of the cache
private:
	int						EnsureCanStore(int Required);				// when we push new data into the list we should make sure it can fit into our store buffer
	//member variables
	int						HaveBuffToStore;
	DataCollectionHeader	*Data;
};

enum DataCollectionIteratorReturnCodes
{
	DCI_SUCCESS = 0,
	DCI_ERROR = 1,
	DCI_NO_MORE_DATA,
};

class DataCollectionIterator
{
public:
	DataCollectionIterator();
	void Init(GenericDataStore *IterateWhat)
	{
		Init(IterateWhat->GetData());
	}
	void Init(DataCollectionHeader *IterateWhat);						// set internal state to the beggining of a list of data blocks
	int GetNext(char **Data, int &Size, int &Type);						// jump to the beggining of 
private:
	DataCollectionHeader	*IterateWhat;
	int						AtBlock;
	unsigned char			*NextData;
};
/// @endcond