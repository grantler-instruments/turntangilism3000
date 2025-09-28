#!/bin/sh

scp -r ../tamputer/* root@bela.local:/root/Bela/projects/tamputer/
scp -r ../bela_io_tester root@bela.local:/root/Bela/projects/
scp -r ../bela_headphone_tester root@bela.local:/root/Bela/projects/
# scp -r ../bela_test/* root@bela.local:/root/Bela/projects/bela_test/
scp -r ../scripts/* root@bela.local:/root/Bela/scripts/
scp -r ../../samples/drums root@bela.local:/root/Bela/samples/drums