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

# Bootstrap

CFGFLAGS = --enable-gtk-doc  --enable-gtk-doc-pdf --enable-gcc-warnings

ifeq ($(.DEFAULT_GOAL),abort-due-to-no-makefile)
.DEFAULT_GOAL := bootstrap
endif

autoreconf:
	touch ChangeLog
	test -f ./configure || autoreconf --install

bootstrap: autoreconf
	./configure $(CFGFLAGS)
	make

# Various settings

INDENT_SOURCES = `find . -name '*.[ch]' | grep -v -e /gl/ -e build-aux`

update-copyright-env = UPDATE_COPYRIGHT_HOLDER="Yubico AB" UPDATE_COPYRIGHT_USE_INTERVALS=1

local-checks-to-skip = sc_GPL_version sc_bindtextdomain

exclude_file_name_regexp--sc_avoid_if_before_free = ^gl/
exclude_file_name_regexp--sc_cast_of_alloca_return_value = ^gl/

# Maintainer rules

glimport:
	gnulib-tool --add-import

ChangeLog:
	cd $(srcdir) && git2cl > ChangeLog

my-release:
	@if test -z "$(KEYID)"; then \
		echo "Try this instead:"; \
		echo "  make release KEYID=[PGPKEYID]"; \
		echo "For example:"; \
		echo "  make release KEYID=2117364A"; \
		exit 1; \
	fi
	@if test ! -d "$(YUBICO_GITHUB_REPO)"; then \
		echo "yubico.github.com repo not found!"; \
		echo "Make sure that YUBICO_GITHUB_REPO is set"; \
		exit 1; \
	fi
	@head -3 $(srcdir)/NEWS | grep -q "Version $(VERSION) .released `date -I`" || \
		(echo 'error: You need to update date/version in $(srcdir)/NEWS'; exit 1)
	rm -f $(srcdir)/ChangeLog
	make ChangeLog distcheck
	gpg --detach-sign --default-key $(KEYID) $(PACKAGE)-$(VERSION).tar.gz
	gpg --verify $(PACKAGE)-$(VERSION).tar.gz.sig
	cd $(srcdir) && git push
	cd $(srcdir) && git tag -u $(KEYID) -m $(VERSION) $(PACKAGE)-$(VERSION)
	cd $(srcdir) && git push --tags
	$(YUBICO_GITHUB_REPO)/publish $(PACKAGE) $(VERSION) $(PACKAGE)-$(VERSION).tar.gz*
	rsync -a $(srcdir)/gtk-doc/html $(YUBICO_GITHUB_REPO)/libykneomgr/gtk-doc/

release: my-release
