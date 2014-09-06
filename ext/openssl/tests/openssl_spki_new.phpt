--TEST--
Testing openssl_spki_new()
Tests SPKAC for all available private key sizes & hashing algorithms
--SKIPIF--
<?php
if (!extension_loaded("openssl")) die("skip");
if (!@openssl_pkey_new()) die("skip cannot create private key");
?>
--FILE--
<?php

/* array of private key sizes to test */
$ksize = array('1024'=>1024,
			   '2048'=>2048,
			   '4096'=>4096);

/* array of available hashings to test */
$algo = array('md4'=>OPENSSL_ALGO_MD4,
              'md5'=>OPENSSL_ALGO_MD5,
              'sha1'=>OPENSSL_ALGO_SHA1,
			  'sha224'=>OPENSSL_ALGO_SHA224,
              'sha256'=>OPENSSL_ALGO_SHA256,
              'sha384'=>OPENSSL_ALGO_SHA384,
              'sha512'=>OPENSSL_ALGO_SHA512,
              'rmd160'=>OPENSSL_ALGO_RMD160);

/* loop over key sizes for test */
foreach($ksize as $k => $v) {

	/* generate new private key of specified size to use for tests */
	$pkey = openssl_pkey_new(array('digest_alg' => 'sha512',
	                               'private_key_type' => OPENSSL_KEYTYPE_RSA,
	                               'private_key_bits' => $v));
	openssl_pkey_export($pkey, $pass);

	/* loop to create and verify results */
	foreach($algo as $key => $value) {
		var_dump(openssl_spki_new($pkey, _uuid(), $value));
	}
	openssl_free_key($pkey);
}

/* generate a random challenge */
function _uuid()
{
 return sprintf('%04x%04x-%04x-%04x-%04x-%04x%04x%04x', mt_rand(0, 0xffff),
                mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0x0fff) | 0x4000,
                mt_rand(0, 0x3fff) | 0x8000, mt_rand(0, 0xffff),
                mt_rand(0, 0xffff), mt_rand(0, 0xffff));
}

?>
--EXPECTF--
string(478) "%s"
string(478) "%s"
string(478) "%s"
string(478) "%s"
string(478) "%s"
string(478) "%s"
string(478) "%s"
string(474) "%s"
string(830) "%s"
string(830) "%s"
string(830) "%s"
string(830) "%s"
string(830) "%s"
string(830) "%s"
string(830) "%s"
string(826) "%s"
string(1510) "%s"
string(1510) "%s"
string(1510) "%s"
string(1510) "%s"
string(1510) "%s"
string(1510) "%s"
string(1510) "%s"
string(1506) "%s"
