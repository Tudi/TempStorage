using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System;
using System.Security.Cryptography;

namespace SAFFileHandler
{
    public static class SAFCrypto
    {
        static byte[] SAFEncryptionKey = new byte[] { 0xAE, 0xEC, 0x5E, 0x77, 0x92, 0xFF, 0x0D, 0x37, 0xE0, 0xA3, 0x82, 0x77, 0x20, 0x5A, 0xC4, 0xCC, 0xE7, 0x83, 0xD8, 0xE9, 0x49, 0x8D, 0x87, 0x38, 0x00, 0xB5, 0x9B, 0xE9, 0x5C, 0x1B, 0xE5, 0x7F };
        const int CryptoPP_AES_BLOCKSIZE = 16;
        const int CryptoPP_SHA256_DIGESTSIZE = 32;
        public static void Decrypt1Block_CBC_AES_256_NOAUTH(byte[] iv, byte[] data, int dataLen, byte[] out_data)
        {
            using (Aes aes = Aes.Create())
            {
                aes.Mode = CipherMode.CBC;
                aes.KeySize = 256;
                aes.BlockSize = 128;
                aes.Padding = PaddingMode.None;
                aes.Key = SAFEncryptionKey;
                aes.IV = iv;

                using (ICryptoTransform decryptor = aes.CreateDecryptor())
                {
                    decryptor.TransformBlock(data, 0, dataLen, out_data, 0);
                }
            }
        }
        public static void GetDataBlockHash(byte[] iv, byte[] data, int dataSize, byte[] hash)
        {
            using (HMACSHA256 hmac = new HMACSHA256(SAFEncryptionKey))
            {
                hmac.TransformBlock(iv, 0, iv.Length, iv, 0); // Hash the first block of data
                hmac.TransformBlock(data, 0, dataSize, data, 0); // Hash the second block of data
                hmac.TransformFinalBlock(new byte[0], 0, 0); // Finalize the hash
                if (hmac.Hash != null)
                {
                    Array.Copy(hmac.Hash, hash, hash.Length); // Copy the result to the output array
                }
            }
        }

        public static int ReadGenericEncryptedBlock(FileStream f, int blockSize, byte[] out_dec)
        {
            byte[] iv = new byte[CryptoPP_AES_BLOCKSIZE];
            byte[] hash = new byte[CryptoPP_SHA256_DIGESTSIZE];
            byte[] hash_check = new byte[CryptoPP_SHA256_DIGESTSIZE];

            int readCount;
            readCount = f.Read(iv, 0, CryptoPP_AES_BLOCKSIZE);
            if (readCount != CryptoPP_AES_BLOCKSIZE)
            {
                Console.WriteLine("ERROR: File does not contain enough data to read IV");
                return -1;
            }

            byte[] tempEncData = new byte[blockSize];
            readCount = f.Read(tempEncData, 0, blockSize);
            if (readCount != blockSize)
            {
                Console.WriteLine("File does not contain enough data to read block data");
                return -2;
            }

            readCount = f.Read(hash, 0, CryptoPP_SHA256_DIGESTSIZE);
            if (readCount != CryptoPP_SHA256_DIGESTSIZE)
            {
                Console.WriteLine("File does not contain enough data to read hash data");
                return -2;
            }

            Decrypt1Block_CBC_AES_256_NOAUTH(iv, tempEncData, blockSize, out_dec);

            GetDataBlockHash(iv, tempEncData, blockSize, hash_check);
            if (!hash.SequenceEqual(hash_check))
            {
                Console.WriteLine($"Data authentication failed for block sized {blockSize}");
            }

            return 0;
        }

        public static int ReadGenericBlock(FileStream f, int blockSize, byte[] out_dec)
        {
            byte[] hash = new byte[CryptoPP_SHA256_DIGESTSIZE];
            byte[] hash_check = new byte[CryptoPP_SHA256_DIGESTSIZE];

            int readCount = f.Read(out_dec, 0, blockSize);
            if (readCount != blockSize)
            {
                Console.WriteLine("File does not contain enough data to read block data");
                return -2;
            }

            readCount = f.Read(hash, 0, hash.Length);
            if (readCount != hash.Length)
            {
                Console.WriteLine("File does not contain enough data to read hash data");
                return -2;
            }

            using (SHA256 sha256 = SHA256.Create())
            {
                byte[] hashBytes = sha256.ComputeHash(out_dec, 0, blockSize); // Hash the first block of data
                Buffer.BlockCopy(hashBytes, 0, hash_check, 0, hashBytes.Length); // copy the hash value to hash_check
            }

            if (!hash.SequenceEqual(hash_check))
            {
                Console.WriteLine("Data authentication failed for block sized {0}", blockSize);
            }

            return 0;
        }

        public static void GenerateRandomBytes(byte[] buffer)
        {
            using (var rng = RandomNumberGenerator.Create())
            {
                rng.GetBytes(buffer);
            }
        }

        public static void Encrypt1Block_CBC_AES_256_NOAUTH(byte[] iv, byte[] data, int dataLength, byte[] out_data)
        {
            using (var aes = Aes.Create())
            {
                aes.Mode = CipherMode.CBC;
                aes.Padding = PaddingMode.PKCS7;
                aes.KeySize = 256;
                aes.BlockSize = 128;
                aes.Key = SAFEncryptionKey;

                using (var encryptor = aes.CreateEncryptor(aes.Key, iv))
                {
                    encryptor.TransformBlock(data, 0, dataLength, out_data, 0);
                }
            }
        }

        public static void WriteGenericEncryptedBlock(FileStream f, byte[] buff, int bufferSize)
        {
            byte[] iv = new byte[CryptoPP_AES_BLOCKSIZE];
            GenerateRandomBytes(iv);
            byte[] ciphertext = new byte[bufferSize];
            Encrypt1Block_CBC_AES_256_NOAUTH(iv, buff, bufferSize, ciphertext);

            byte[] hash = new byte[CryptoPP_SHA256_DIGESTSIZE];
            GetDataBlockHash(iv, ciphertext, ciphertext.Length, hash);

            f.Write(iv, 0, iv.Length);
            f.Write(ciphertext, 0, ciphertext.Length);
            f.Write(hash, 0, hash.Length);
        }

        public static void WriteGenericBlock(FileStream f, byte[] buff, int bufferSize)
        {
            byte[]? hash = null;
            using (var sha256 = SHA256.Create())
            {
                hash = sha256.ComputeHash(buff, 0, bufferSize);
            }

            f.Write(buff, 0, bufferSize);
            f.Write(hash, 0, hash.Length);
        }
    }
}
