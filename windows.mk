# Copyright (C) 2013-2014 Yubico AB
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PACKAGE=libykneomgr
ZLIB_VERSION=1.2.8
LIBZIP_VERSION=0.11.2

all: usage 32bit 64bit

.PHONY: usage
usage:
	@if test -z "$(VERSION)" || test -z "$(PGPKEYID)"; then \
		echo "Try this instead:"; \
		echo "  make PGPKEYID=[PGPKEYID] VERSION=[VERSION]"; \
		echo "For example:"; \
		echo "  make PGPKEYID=2117364A VERSION=1.6.0"; \
		exit 1; \
	fi

doit:
	rm -rf tmp$(ARCH) && mkdir tmp$(ARCH) && cd tmp$(ARCH) && \
	mkdir -p root/licenses && \
	cp ../zlib-$(ZLIB_VERSION).tar.gz . || \
	wget "http://zlib.net/zlib-$(ZLIB_VERSION).tar.gz" && \
	tar xfa zlib-$(ZLIB_VERSION).tar.gz && \
	cd zlib-$(ZLIB_VERSION) && \
	make -f win32/Makefile.gcc install DESTDIR=$(PWD)/tmp$(ARCH)/root/ PREFIX=$(HOST)- INCLUDE_PATH=include LIBRARY_PATH=lib BINARY_PATH=bin && \
	cd .. && \
	cp ../libzip-$(LIBZIP_VERSION).tar.gz . || \
	wget "http://www.nih.at/libzip/libzip-$(LIBZIP_VERSION).tar.gz" && \
	tar xfa libzip-$(LIBZIP_VERSION).tar.gz && \
	cd libzip-$(LIBZIP_VERSION) && \
	lt_cv_deplibs_check_method=pass_all ./configure --host=$(HOST) --build=x86_64-unknown-linux-gnu --prefix=$(PWD)/tmp$(ARCH)/root --with-zlib=$(PWD)/tmp$(ARCH)/root && \
	make install && \
	rm -f $(PWD)/tmp$(ARCH)/root/bin/zipcmp.exe $(PWD)/tmp$(ARCH)/root/bin/zipmerge.exe $(PWD)/tmp$(ARCH)/root/bin/ziptorrent.exe && \
	cp LICENSE $(PWD)/tmp$(ARCH)/root/licenses/libzip.txt && \
	cd .. && \
	cp ../$(PACKAGE)-$(VERSION).tar.gz . && \
	tar xfa $(PACKAGE)-$(VERSION).tar.gz && \
	cd $(PACKAGE)-$(VERSION)/ && \
	lt_cv_deplibs_check_method=pass_all PKG_CONFIG_PATH=$(PWD)/tmp$(ARCH)/root/lib/pkgconfig ./configure --host=$(HOST) --build=x86_64-unknown-linux-gnu --prefix=$(PWD)/tmp$(ARCH)/root --enable-gtk-doc --enable-gtk-doc-pdf && \
	make install $(CHECK) && \
	rm -rf $(PWD)/tmp$(ARCH)/root/lib/pkgconfig/ && \
	mkdir $(PWD)/tmp$(ARCH)/root/doc && \
	cp gtk-doc/$(PACKAGE).pdf $(PWD)/tmp$(ARCH)/root/doc/ && \
	cp COPYING $(PWD)/tmp$(ARCH)/root/licenses/$(PACKAGE).txt && \
	cd .. && \
	cd root && \
	mv share/gtk-doc/html/$(PACKAGE)/* $(PWD)/tmp$(ARCH)/root/doc/ && \
	rm -rf share/gtk-doc && \
	rm -f ../../$(PACKAGE)-$(VERSION)-win$(ARCH).zip && \
	zip -r ../../$(PACKAGE)-$(VERSION)-win$(ARCH).zip *

32bit:
	$(MAKE) -f windows.mk doit ARCH=32 HOST=i686-w64-mingw32 CHECK=check

64bit:
	$(MAKE) -f windows.mk doit ARCH=64 HOST=x86_64-w64-mingw32

upload:
	@if test ! -d "$(YUBICO_GITHUB_REPO)"; then \
		echo "yubico.github.com repo not found!"; \
		echo "Make sure that YUBICO_GITHUB_REPO is set"; \
		exit 1; \
	fi
	gpg --detach-sign --default-key $(PGPKEYID) \
		$(PACKAGE)-$(VERSION)-win$(BITS).zip
	gpg --verify $(PACKAGE)-$(VERSION)-win$(BITS).zip.sig
	$(YUBICO_GITHUB_REPO)/publish $(PACKAGE) $(VERSION) $(PACKAGE)-$(VERSION)-win${BITS}.zip*

upload-32bit:
	$(MAKE) -f windows.mk upload BITS=32

upload-64bit:
	$(MAKE) -f windows.mk upload BITS=64
