#!/usr/bin/env bash
#gcc -o mpwget mpwget.c && ./mpwget fperez/reto1.txt fperez/reto2.txt fperez/reto1.jpg :www.datsi.fi.upm.es :www.datsi.fi.upm.es :laurel.datsi.fi.upm.es
gcc -o mpwget mpwget.c -lcurl
./mpwget fperez/reto1.txt fperez/reto2.txt :www.datsi.fi.upm.es :laurel.datsi.fi.upm.es :laurel.datsi.fi.upm.es :www.datsi.fi.upm.es :laurel.datsi.fi.upm.es :laurel.datsi.fi.upm.es :laurel.datsi.fi.upm.es