#ifdef BUILD_CLIENT_PACKETS
    #include "StdAfx.h"
    #include <windows.h>
#else
    #include <windows.h>
#endif
#include <stdint.h>

static char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/' };
static int mod_table[] = { 0, 2, 1 };


char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length)
{
    *output_length = 4 * ((input_length + 2) / 3);

    char* encoded_data = (char*)malloc(4 * input_length + 1);
    if (encoded_data == NULL)
    {
        return NULL;
    }

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
    {
        encoded_data[*output_length - 1 - i] = '=';
    }

    encoded_data[*output_length] = 0;

    return encoded_data;
}

static char* decoding_table = NULL;
void build_decoding_table() {

    decoding_table = (char*)malloc(256);
    if (decoding_table == NULL)
    {
        return;
    }

    for (int i = 0; i < 64; i++)
    {
        decoding_table[(unsigned char)encoding_table[i]] = (char)i;
    }
}

unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length)
{

    if (decoding_table == NULL)
    {
        build_decoding_table();
    }

    if (input_length % 4 != 0)
    {
        return NULL;
    }

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=')
    {
        (*output_length)--;
    }
    if (data[input_length - 2] == '=')
    {
        (*output_length)--;
    }

    unsigned char* decoded_data = (unsigned char*)malloc(*output_length + 4);
    if (decoded_data == NULL)
    {
        return NULL;
    }

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
            + (sextet_b << 2 * 6)
            + (sextet_c << 1 * 6)
            + (sextet_d << 0 * 6);

        if (j < *output_length)
        {
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        }
        if (j < *output_length)
        {
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        }
        if (j < *output_length)
        {
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
        }
    }

    decoded_data[*output_length] = 0;

    return decoded_data;
}

__int64 GetNanoStampWindows()
{
    static LARGE_INTEGER frequency = { 0 };
    LARGE_INTEGER counter;

    // Get the frequency of the performance counter
    if (frequency.QuadPart == 0 &&  !QueryPerformanceFrequency(&frequency))
    {
        // Handle error - the system may not support high-resolution performance counter
        return -1;
    }

    // Get the current value of the performance counter
    if (!QueryPerformanceCounter(&counter))
    {
        // Handle error - failed to retrieve the counter value
        return -1;
    }

    // Calculate the time in nanoseconds
    int64_t nanoseconds = (int64_t)((double)counter.QuadPart * 1e9 / frequency.QuadPart);

    return nanoseconds;
}

static const uint64_t crc64_tab[256] = {
    (uint64_t)0x0000000000000000, (uint64_t)0x7ad870c830358979,
    (uint64_t)0xf5b0e190606b12f2, (uint64_t)0x8f689158505e9b8b,
    (uint64_t)0xc038e5739841b68f, (uint64_t)0xbae095bba8743ff6,
    (uint64_t)0x358804e3f82aa47d, (uint64_t)0x4f50742bc81f2d04,
    (uint64_t)0xab28ecb46814fe75, (uint64_t)0xd1f09c7c5821770c,
    (uint64_t)0x5e980d24087fec87, (uint64_t)0x24407dec384a65fe,
    (uint64_t)0x6b1009c7f05548fa, (uint64_t)0x11c8790fc060c183,
    (uint64_t)0x9ea0e857903e5a08, (uint64_t)0xe478989fa00bd371,
    (uint64_t)0x7d08ff3b88be6f81, (uint64_t)0x07d08ff3b88be6f8,
    (uint64_t)0x88b81eabe8d57d73, (uint64_t)0xf2606e63d8e0f40a,
    (uint64_t)0xbd301a4810ffd90e, (uint64_t)0xc7e86a8020ca5077,
    (uint64_t)0x4880fbd87094cbfc, (uint64_t)0x32588b1040a14285,
    (uint64_t)0xd620138fe0aa91f4, (uint64_t)0xacf86347d09f188d,
    (uint64_t)0x2390f21f80c18306, (uint64_t)0x594882d7b0f40a7f,
    (uint64_t)0x1618f6fc78eb277b, (uint64_t)0x6cc0863448deae02,
    (uint64_t)0xe3a8176c18803589, (uint64_t)0x997067a428b5bcf0,
    (uint64_t)0xfa11fe77117cdf02, (uint64_t)0x80c98ebf2149567b,
    (uint64_t)0x0fa11fe77117cdf0, (uint64_t)0x75796f2f41224489,
    (uint64_t)0x3a291b04893d698d, (uint64_t)0x40f16bccb908e0f4,
    (uint64_t)0xcf99fa94e9567b7f, (uint64_t)0xb5418a5cd963f206,
    (uint64_t)0x513912c379682177, (uint64_t)0x2be1620b495da80e,
    (uint64_t)0xa489f35319033385, (uint64_t)0xde51839b2936bafc,
    (uint64_t)0x9101f7b0e12997f8, (uint64_t)0xebd98778d11c1e81,
    (uint64_t)0x64b116208142850a, (uint64_t)0x1e6966e8b1770c73,
    (uint64_t)0x8719014c99c2b083, (uint64_t)0xfdc17184a9f739fa,
    (uint64_t)0x72a9e0dcf9a9a271, (uint64_t)0x08719014c99c2b08,
    (uint64_t)0x4721e43f0183060c, (uint64_t)0x3df994f731b68f75,
    (uint64_t)0xb29105af61e814fe, (uint64_t)0xc849756751dd9d87,
    (uint64_t)0x2c31edf8f1d64ef6, (uint64_t)0x56e99d30c1e3c78f,
    (uint64_t)0xd9810c6891bd5c04, (uint64_t)0xa3597ca0a188d57d,
    (uint64_t)0xec09088b6997f879, (uint64_t)0x96d1784359a27100,
    (uint64_t)0x19b9e91b09fcea8b, (uint64_t)0x636199d339c963f2,
    (uint64_t)0xdf7adabd7a6e2d6f, (uint64_t)0xa5a2aa754a5ba416,
    (uint64_t)0x2aca3b2d1a053f9d, (uint64_t)0x50124be52a30b6e4,
    (uint64_t)0x1f423fcee22f9be0, (uint64_t)0x659a4f06d21a1299,
    (uint64_t)0xeaf2de5e82448912, (uint64_t)0x902aae96b271006b,
    (uint64_t)0x74523609127ad31a, (uint64_t)0x0e8a46c1224f5a63,
    (uint64_t)0x81e2d7997211c1e8, (uint64_t)0xfb3aa75142244891,
    (uint64_t)0xb46ad37a8a3b6595, (uint64_t)0xceb2a3b2ba0eecec,
    (uint64_t)0x41da32eaea507767, (uint64_t)0x3b024222da65fe1e,
    (uint64_t)0xa2722586f2d042ee, (uint64_t)0xd8aa554ec2e5cb97,
    (uint64_t)0x57c2c41692bb501c, (uint64_t)0x2d1ab4dea28ed965,
    (uint64_t)0x624ac0f56a91f461, (uint64_t)0x1892b03d5aa47d18,
    (uint64_t)0x97fa21650afae693, (uint64_t)0xed2251ad3acf6fea,
    (uint64_t)0x095ac9329ac4bc9b, (uint64_t)0x7382b9faaaf135e2,
    (uint64_t)0xfcea28a2faafae69, (uint64_t)0x8632586aca9a2710,
    (uint64_t)0xc9622c4102850a14, (uint64_t)0xb3ba5c8932b0836d,
    (uint64_t)0x3cd2cdd162ee18e6, (uint64_t)0x460abd1952db919f,
    (uint64_t)0x256b24ca6b12f26d, (uint64_t)0x5fb354025b277b14,
    (uint64_t)0xd0dbc55a0b79e09f, (uint64_t)0xaa03b5923b4c69e6,
    (uint64_t)0xe553c1b9f35344e2, (uint64_t)0x9f8bb171c366cd9b,
    (uint64_t)0x10e3202993385610, (uint64_t)0x6a3b50e1a30ddf69,
    (uint64_t)0x8e43c87e03060c18, (uint64_t)0xf49bb8b633338561,
    (uint64_t)0x7bf329ee636d1eea, (uint64_t)0x012b592653589793,
    (uint64_t)0x4e7b2d0d9b47ba97, (uint64_t)0x34a35dc5ab7233ee,
    (uint64_t)0xbbcbcc9dfb2ca865, (uint64_t)0xc113bc55cb19211c,
    (uint64_t)0x5863dbf1e3ac9dec, (uint64_t)0x22bbab39d3991495,
    (uint64_t)0xadd33a6183c78f1e, (uint64_t)0xd70b4aa9b3f20667,
    (uint64_t)0x985b3e827bed2b63, (uint64_t)0xe2834e4a4bd8a21a,
    (uint64_t)0x6debdf121b863991, (uint64_t)0x1733afda2bb3b0e8,
    (uint64_t)0xf34b37458bb86399, (uint64_t)0x8993478dbb8deae0,
    (uint64_t)0x06fbd6d5ebd3716b, (uint64_t)0x7c23a61ddbe6f812,
    (uint64_t)0x3373d23613f9d516, (uint64_t)0x49aba2fe23cc5c6f,
    (uint64_t)0xc6c333a67392c7e4, (uint64_t)0xbc1b436e43a74e9d,
    (uint64_t)0x95ac9329ac4bc9b5, (uint64_t)0xef74e3e19c7e40cc,
    (uint64_t)0x601c72b9cc20db47, (uint64_t)0x1ac40271fc15523e,
    (uint64_t)0x5594765a340a7f3a, (uint64_t)0x2f4c0692043ff643,
    (uint64_t)0xa02497ca54616dc8, (uint64_t)0xdafce7026454e4b1,
    (uint64_t)0x3e847f9dc45f37c0, (uint64_t)0x445c0f55f46abeb9,
    (uint64_t)0xcb349e0da4342532, (uint64_t)0xb1eceec59401ac4b,
    (uint64_t)0xfebc9aee5c1e814f, (uint64_t)0x8464ea266c2b0836,
    (uint64_t)0x0b0c7b7e3c7593bd, (uint64_t)0x71d40bb60c401ac4,
    (uint64_t)0xe8a46c1224f5a634, (uint64_t)0x927c1cda14c02f4d,
    (uint64_t)0x1d148d82449eb4c6, (uint64_t)0x67ccfd4a74ab3dbf,
    (uint64_t)0x289c8961bcb410bb, (uint64_t)0x5244f9a98c8199c2,
    (uint64_t)0xdd2c68f1dcdf0249, (uint64_t)0xa7f41839ecea8b30,
    (uint64_t)0x438c80a64ce15841, (uint64_t)0x3954f06e7cd4d138,
    (uint64_t)0xb63c61362c8a4ab3, (uint64_t)0xcce411fe1cbfc3ca,
    (uint64_t)0x83b465d5d4a0eece, (uint64_t)0xf96c151de49567b7,
    (uint64_t)0x76048445b4cbfc3c, (uint64_t)0x0cdcf48d84fe7545,
    (uint64_t)0x6fbd6d5ebd3716b7, (uint64_t)0x15651d968d029fce,
    (uint64_t)0x9a0d8ccedd5c0445, (uint64_t)0xe0d5fc06ed698d3c,
    (uint64_t)0xaf85882d2576a038, (uint64_t)0xd55df8e515432941,
    (uint64_t)0x5a3569bd451db2ca, (uint64_t)0x20ed197575283bb3,
    (uint64_t)0xc49581ead523e8c2, (uint64_t)0xbe4df122e51661bb,
    (uint64_t)0x3125607ab548fa30, (uint64_t)0x4bfd10b2857d7349,
    (uint64_t)0x04ad64994d625e4d, (uint64_t)0x7e7514517d57d734,
    (uint64_t)0xf11d85092d094cbf, (uint64_t)0x8bc5f5c11d3cc5c6,
    (uint64_t)0x12b5926535897936, (uint64_t)0x686de2ad05bcf04f,
    (uint64_t)0xe70573f555e26bc4, (uint64_t)0x9ddd033d65d7e2bd,
    (uint64_t)0xd28d7716adc8cfb9, (uint64_t)0xa85507de9dfd46c0,
    (uint64_t)0x273d9686cda3dd4b, (uint64_t)0x5de5e64efd965432,
    (uint64_t)0xb99d7ed15d9d8743, (uint64_t)0xc3450e196da80e3a,
    (uint64_t)0x4c2d9f413df695b1, (uint64_t)0x36f5ef890dc31cc8,
    (uint64_t)0x79a59ba2c5dc31cc, (uint64_t)0x037deb6af5e9b8b5,
    (uint64_t)0x8c157a32a5b7233e, (uint64_t)0xf6cd0afa9582aa47,
    (uint64_t)0x4ad64994d625e4da, (uint64_t)0x300e395ce6106da3,
    (uint64_t)0xbf66a804b64ef628, (uint64_t)0xc5bed8cc867b7f51,
    (uint64_t)0x8aeeace74e645255, (uint64_t)0xf036dc2f7e51db2c,
    (uint64_t)0x7f5e4d772e0f40a7, (uint64_t)0x05863dbf1e3ac9de,
    (uint64_t)0xe1fea520be311aaf, (uint64_t)0x9b26d5e88e0493d6,
    (uint64_t)0x144e44b0de5a085d, (uint64_t)0x6e963478ee6f8124,
    (uint64_t)0x21c640532670ac20, (uint64_t)0x5b1e309b16452559,
    (uint64_t)0xd476a1c3461bbed2, (uint64_t)0xaeaed10b762e37ab,
    (uint64_t)0x37deb6af5e9b8b5b, (uint64_t)0x4d06c6676eae0222,
    (uint64_t)0xc26e573f3ef099a9, (uint64_t)0xb8b627f70ec510d0,
    (uint64_t)0xf7e653dcc6da3dd4, (uint64_t)0x8d3e2314f6efb4ad,
    (uint64_t)0x0256b24ca6b12f26, (uint64_t)0x788ec2849684a65f,
    (uint64_t)0x9cf65a1b368f752e, (uint64_t)0xe62e2ad306bafc57,
    (uint64_t)0x6946bb8b56e467dc, (uint64_t)0x139ecb4366d1eea5,
    (uint64_t)0x5ccebf68aecec3a1, (uint64_t)0x2616cfa09efb4ad8,
    (uint64_t)0xa97e5ef8cea5d153, (uint64_t)0xd3a62e30fe90582a,
    (uint64_t)0xb0c7b7e3c7593bd8, (uint64_t)0xca1fc72bf76cb2a1,
    (uint64_t)0x45775673a732292a, (uint64_t)0x3faf26bb9707a053,
    (uint64_t)0x70ff52905f188d57, (uint64_t)0x0a2722586f2d042e,
    (uint64_t)0x854fb3003f739fa5, (uint64_t)0xff97c3c80f4616dc,
    (uint64_t)0x1bef5b57af4dc5ad, (uint64_t)0x61372b9f9f784cd4,
    (uint64_t)0xee5fbac7cf26d75f, (uint64_t)0x9487ca0fff135e26,
    (uint64_t)0xdbd7be24370c7322, (uint64_t)0xa10fceec0739fa5b,
    (uint64_t)0x2e675fb4576761d0, (uint64_t)0x54bf2f7c6752e8a9,
    (uint64_t)0xcdcf48d84fe75459, (uint64_t)0xb71738107fd2dd20,
    (uint64_t)0x387fa9482f8c46ab, (uint64_t)0x42a7d9801fb9cfd2,
    (uint64_t)0x0df7adabd7a6e2d6, (uint64_t)0x772fdd63e7936baf,
    (uint64_t)0xf8474c3bb7cdf024, (uint64_t)0x829f3cf387f8795d,
    (uint64_t)0x66e7a46c27f3aa2c, (uint64_t)0x1c3fd4a417c62355,
    (uint64_t)0x935745fc4798b8de, (uint64_t)0xe98f353477ad31a7,
    (uint64_t)0xa6df411fbfb21ca3, (uint64_t)0xdc0731d78f8795da,
    (uint64_t)0x536fa08fdfd90e51, (uint64_t)0x29b7d047efec8728,
};

uint64_t crc64(uint64_t crc, const void* buffer, uint64_t size)
{
    uint64_t j;
    const unsigned char* s = (unsigned char*)buffer;
    for (j = 0; j < size; j++) {
        uint8_t byte = s[j];
        uint8_t index = (uint8_t)crc ^ byte;
        crc = crc64_tab[index] ^ (crc >> 8);
    }
    return crc;
}

#define FNV_PRIME_64 0x100000001b3ULL
#define FNV_OFFSET_64 0xcbf29ce484222325ULL

uint64_t fnv1a_hash(uint64_t hash, const void* data, size_t size) 
{
    const unsigned char* s = (const unsigned char*)data;

    for (size_t i = 0; i < size; i++) {
        hash ^= s[i];
        hash *= FNV_PRIME_64;
    }

    return hash;
}

uint32_t crc31_hash(int seed, const void* data, size_t size)
{
    const unsigned char* s = (const unsigned char*)data;
    int crc = seed & 0x7FFFFFFF; // Initialize with all bits set to 1 except the sign bit

    for (size_t i = 0; i < size; i++) {
        crc = crc ^ s[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0x4C11DB7 & (-(int)(crc & 1)));
        }
    }

    return crc & 0x7FFFFFFF; // Mask to keep only the lower 31 bits
}

void GetTaskbarPosAndSize(int& x, int& y, int& width, int& height)
{
    APPBARDATA appBarData;
    appBarData.cbSize = sizeof(APPBARDATA);

    if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData)) 
    {
        // The taskbar position and size are now in the appBarData structure.
        // You can access them like this:
        int taskbarLeft = appBarData.rc.left;
        int taskbarTop = appBarData.rc.top;
        int taskbarRight = appBarData.rc.right;
        int taskbarBottom = appBarData.rc.bottom;

        // You can then use these values as needed.
        // For example, to get the width and height of the taskbar:
        int taskbarWidth = taskbarRight - taskbarLeft;
        int taskbarHeight = taskbarBottom - taskbarTop;

        x = taskbarLeft;
        y = taskbarTop;
        width = taskbarWidth;
        height = taskbarHeight;
    }
    else {
        x = y = width = height = 0;
    }
}

void AutoSizeConsoleWindow(int ApplicationIndex = 0, int ExpectedApplications = 1)
{
    system("mode 650");
    // Get handle to the desktop window
    HWND desktop = GetDesktopWindow();
    HANDLE StdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get desktop window dimensions
    RECT desktopRect;
    GetClientRect(desktop, &desktopRect);
    int desktopWidth = desktopRect.right - desktopRect.left;
    int desktopHeight = desktopRect.bottom - desktopRect.top - 60;
    
    // Get the size of a single character in the console font
    CONSOLE_FONT_INFO cfi;
    GetCurrentConsoleFont(StdHandle, FALSE, &cfi);
    int fontWidth = cfi.dwFontSize.X;
    int fontHeight = cfi.dwFontSize.Y;

    // Calculate the console window width and height in characters
    SHORT consoleWidth = (SHORT)(desktopWidth / fontWidth);
    SHORT consoleHeight = (SHORT)(desktopHeight / (fontHeight * ExpectedApplications));

    // Set the new console window size and position
    COORD coord;
    coord.X = consoleWidth * 2;
    coord.Y = consoleHeight * 8;
    SetConsoleScreenBufferSize(StdHandle, coord);

    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = consoleWidth - 1;
    rect.Bottom = consoleHeight - 1;
    SetConsoleWindowInfo(StdHandle, TRUE, &rect);

    HWND console = GetConsoleWindow();
    int ConsoleHeight = (desktopHeight / ExpectedApplications);
    SetWindowPos(console, 0, 
        0, ConsoleHeight * ApplicationIndex,
        desktopWidth, ConsoleHeight, SWP_NOZORDER);
}