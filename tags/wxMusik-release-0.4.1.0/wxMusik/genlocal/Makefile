# this is the makefile for generating PROJECT_NAME message catalog file and
# building lang.mo files from the translated lang.po catalogs

# this makefile may be invoked to build either PROJECT_NAME.po or any lang.mo

# Autodetect the languages we support.  Currently this relies on make
# being called with this dir as the cwd, but if we generate this file
# with configure an explicit path should be specified -- RL.
PROJECT_NAME=wxMusik

WX_LINGUAS := `ls *.po 2> /dev/null | sed  's/\(\).po/\1/'| sed 's/$(PROJECT_NAME)//'`

# the programs we use (TODO: use configure to detect them)
MSGFMT=msgfmt --verbose
MSGMERGE=msgmerge
XGETTEXT=xgettext
XARGS=xargs -Ixxx echo "\"xxx\""|xargs 


# common xgettext args: C++ syntax, use the specified macro names as markers
XGETTEXT_ARGS=-C -k_ -kwxGetTranslation -kwxTRANSLATE -s -j

# implicit rules
%.mo: %.po
	$(MSGFMT) -o $@ $<

# a PO file must be updated from $(PROJECT_NAME).po to include new translations
%.po: $(PROJECT_NAME).po
	if [ -f $@ ]; then $(MSGMERGE) $@ $(PROJECT_NAME).po > $@.new && mv $@.new $@; else cp $(PROJECT_NAME).po $@; fi

$(PROJECT_NAME).po:
	touch $@
	find .. -name "*.h" | $(XARGS) $(XGETTEXT) $(XGETTEXT_ARGS) -o $(PROJECT_NAME).po
	find .. -name "*.cpp" | $(XARGS) $(XGETTEXT) $(XGETTEXT_ARGS) -o $(PROJECT_NAME).po

allpo: force-update
	@-for t in $(WX_LINGUAS); do $(MAKE) $$t.po; done

allmo:
	@for t in $(WX_LINGUAS); do \
		$(MAKE) $$t.mo; \
		mkdir -p ../locale/$$t; \
		mv -f $$t.mo ../locale/$$t/$(PROJECT_NAME).mo; \
	done

force-update:
	$(RM) $(PROJECT_NAME).po

# print out the percentage of the translated strings
stats: FORCE
	@for i in $(WX_LINGUAS); do \
		x=`$(MSGFMT) -o /dev/null "$$i.po" 2>&1 | sed -e 's/[,\.]//g' \
			-e 's/\([0-9]\+\) translated messages\?/TR=\1/' \
			-e 's/\([0-9]\+\) fuzzy translations\?/FZ=\1/' \
			-e 's/\([0-9]\+\) untranslated messages\?/UT=\1/'`; \
		TR=0 FZ=0 UT=0; \
		eval $$x; \
		TOTAL=`expr $$TR + $$FZ + $$UT`; \
		echo "$$i.po `expr 100 "*" $$TR / $$TOTAL`% of $$TOTAL strings"; \
	done

FORCE:

.PHONY: allpo allmo force-update percentage FORCE

# $Id: Makefile,v 1.15 2004/08/30 17:06:49 gunnar67 Exp $
