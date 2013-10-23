# Copyright (C) 2013 Yubico AB
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SUBDIRS = gl lib src gtk-doc tests

ACLOCAL_AMFLAGS = -I m4 -I gl/m4

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

# Maintainer rules

indent:
	indent `find . -name \*.c -o -name \*.h -o -name \*.h.in`
	indent `find . -name \*.c -o -name \*.h -o -name \*.h.in`

ChangeLog:
	cd $(srcdir) && git2cl > ChangeLog

release:
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
