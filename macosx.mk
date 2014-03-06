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
LIBZIP_VERSION=0.11.2

all: usage doit

.PHONY: usage
usage:
	@if test ! -n "$(VERSION)" || test ! -n "$(PGPKEYID)"; then \
		echo "Try this instead:"; \
		echo "  make PGPKEYID=[PGPKEYID] VERSION=[VERSION]"; \
		echo "For example:"; \
		echo "  make PGPKEYID=2117364A VERSION=1.6.0"; \
		exit 1; \
	fi

doit:
	rm -rf tmp && mkdir tmp && cd tmp && \
	mkdir -p root/licenses && \
	cp ../libzip-$(LIBZIP_VERSION).tar.gz . || \
	wget "http://www.nih.at/libzip/libzip-$(LIBZIP_VERSION).tar.gz" && \
	tar xfz libzip-$(LIBZIP_VERSION).tar.gz && \
	cd libzip-$(LIBZIP_VERSION) && \
	./configure --prefix=$(PWD)/tmp$(ARCH)/root CFLAGS=-mmacosx-version-min=10.6 && \
	make install check && \
	cp LICENSE $(PWD)/tmp/root/licenses/libzip.txt && \
	cd .. && \
	cp ../$(PACKAGE)-$(VERSION).tar.gz . || cp ../../$(PACKAGE)-$(VERSION).tar.gz . && \
	tar xfz $(PACKAGE)-$(VERSION).tar.gz && \
	cd $(PACKAGE)-$(VERSION)/ && \
	PKG_CONFIG_PATH=$(PWD)/tmp/root/lib/pkgconfig ./configure CFLAGS=-mmacosx-version-min=10.6 --prefix=$(PWD)/tmp/root && \
	make install check && \
	rm -f $(PWD)/tmp$(ARCH)/root/bin/zipcmp $(PWD)/tmp$(ARCH)/root/bin/zipmerge $(PWD)/tmp$(ARCH)/root/bin/ziptorrent && \
	rm -rf $(PWD)/tmp/root/lib/pkgconfig/ && \
	install_name_tool -id @executable_path/../lib/libzip.2.dylib $(PWD)/tmp/root/lib/libzip.2.dylib && \
	install_name_tool -change $(PWD)/tmp/root/lib/libzip.2.dylib @executable_path/../lib/libzip.2.dylib $(PWD)/tmp/root/bin/ykneomgr && \
	install_name_tool -id @executable_path/../lib/libykneomgr.0.dylib $(PWD)/tmp/root/lib/libykneomgr.0.dylib && \
	install_name_tool -change $(PWD)/tmp/root/lib/libykneomgr.0.dylib @executable_path/../lib/libykneomgr.0.dylib $(PWD)/tmp/root/bin/ykneomgr && \
	mkdir $(PWD)/tmp/root/doc && \
	cp gtk-doc/$(PACKAGE).pdf $(PWD)/tmp/root/doc/ && \
	cp COPYING $(PWD)/tmp/root/licenses/$(PACKAGE).txt && \
	cd .. && \
	cd root && \
	mv share/gtk-doc/html/$(PACKAGE)/* $(PWD)/tmp/root/doc/ && \
	rm -rf share/gtk-doc && \
	rm -f ../../$(PACKAGE)-$(VERSION)-mac.zip && \
	zip -r ../../$(PACKAGE)-$(VERSION)-mac.zip *

upload:
	@if test ! -d "$(YUBICO_GITHUB_REPO)"; then \
		echo "yubico.github.com repo not found!"; \
		echo "Make sure that YUBICO_GITHUB_REPO is set"; \
		exit 1; \
	fi
	gpg --detach-sign --default-key $(PGPKEYID) \
		$(PACKAGE)-$(VERSION)-mac.zip
	gpg --verify $(PACKAGE)-$(VERSION)-mac.zip.sig
	$(YUBICO_GITHUB_REPO)/publish $(PACKAGE) $(VERSION) $(PACKAGE)-$(VERSION)-mac.zip*
