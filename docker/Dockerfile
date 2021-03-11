FROM ubuntu:20.10

RUN apt-get update && apt-get install -y git build-essential autoconf libxml2-dev vim libssl-dev libz-dev
RUN useradd -ms /bin/bash xar
RUN git clone https://github.com/mackyle/xar.git 
RUN cd /xar/xar/; \
    # Based on https://askubuntu.com/questions/1108506/building-xar-source-error-no-libcrypto :
    sed -i '332s/^.*$/AC_CHECK_LIB([crypto], [OPENSSL_init_crypto], , [have_libcrypto="0"])/' configure.ac; \
    ./autogen.sh; \
    ./configure; \
    make; \
    make install
RUN chown -R xar:xar /xar/
USER xar
WORKDIR /xar/
