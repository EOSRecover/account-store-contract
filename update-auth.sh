#!/bin/sh
cleos_jungle='/usr/local/bin/cleos -u https://jungle4.cryptolions.io'

$cleos_jungle set account permission accountmsig1 active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsig1@owner
$cleos_jungle set account permission accountmsig1 owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsig1@owner

$cleos_jungle set account permission accountmsig2 active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsig2@owner
$cleos_jungle set account permission accountmsig2 owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsig2@owner

$cleos_jungle set account permission accountmsig3 active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsig3@owner
$cleos_jungle set account permission accountmsig3 owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsig3@owner

$cleos_jungle set account permission accountmsig4 active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsig4@owner
$cleos_jungle set account permission accountmsig4 owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsig4@owner

$cleos_jungle set account permission accountmsig5 active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsig5@owner
$cleos_jungle set account permission accountmsig5 owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsig5@owner

$cleos_jungle set account permission accountmsigd active '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' owner -p accountmsigd@owner
$cleos_jungle set account permission accountmsigd owner '{"threshold": 1, "keys": [{"key": "EOS5mkrJue1bYmKp71jwELvHxdFHKf1kee3pLnvHsDqdGo5ijKY9k", "weight": 1}], "accounts": [{"permission":{"actor":"accountmsigy","permission":"active"},"weight":1}]}' -p accountmsigd@owner
