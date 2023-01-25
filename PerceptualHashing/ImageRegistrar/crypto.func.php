<?php
function create_encryption_key() 
{
	return base64_encode(sodium_crypto_secretbox_keygen());
}

/**
 * Encrypt a message
 * 
 * @param string $message - message to encrypt
 * @param string $key - encryption key created using create_encryption_key()
 * @return string
 */
function encrypt($message, $key) 
{
	$key_decoded = base64_decode($key);
	$nonce = random_bytes(SODIUM_CRYPTO_SECRETBOX_NONCEBYTES);

	$cipher = base64_encode($nonce.sodium_crypto_secretbox($message, $nonce, $key_decoded));
	sodium_memzero($message);
	sodium_memzero($key_decoded);
	return $cipher;
}

/**
 * Decrypt a message
 * @param string $encrypted - message encrypted with safeEncrypt()
 * @param string $key - key used for encryption
 * @return string
 */
function decrypt($encrypted, $key) 
{
	$decoded = base64_decode($encrypted);
	$key_decoded = base64_decode($key);
	if ($decoded === false) 
	{
		throw new Exception('Decryption error : the encoding failed');
	}
	if (mb_strlen($decoded, '8bit') < (SODIUM_CRYPTO_SECRETBOX_NONCEBYTES + SODIUM_CRYPTO_SECRETBOX_MACBYTES)) 
	{
		throw new Exception('Decryption error : the message was truncated');
	}
	$nonce = mb_substr($decoded, 0, SODIUM_CRYPTO_SECRETBOX_NONCEBYTES, '8bit');
	$ciphertext = mb_substr($decoded, SODIUM_CRYPTO_SECRETBOX_NONCEBYTES, null, '8bit');

	$plain = sodium_crypto_secretbox_open($ciphertext, $nonce, $key_decoded);
	if ($plain === false) 
	{
		throw new Exception('Decryption error : the message was tampered with in transit');
	}
	sodium_memzero($ciphertext);
	sodium_memzero($key_decoded);
	return $plain;
}
?>