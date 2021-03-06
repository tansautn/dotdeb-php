--TEST--
Phar: copy-on-write test 11 [cache_list]
--INI--
default_charset=UTF-8
phar.cache_list={PWD}/copyonwrite11.phar.php
phar.readonly=0
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--FILE_EXTERNAL--
files/write11.phar
--EXPECTF--
string(174) "<?php
$p = new Phar(__FILE__);
var_dump($p->getStub());
$p2 = new Phar(__FILE__);
$p->setDefaultStub();
echo strlen($p2->getStub()),"\n";
echo "ok\n";
__HALT_COMPILER(); ?>
"
6675
ok