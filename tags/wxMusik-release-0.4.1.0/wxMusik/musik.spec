#
# RPM spec file for Musik
#
# Written by Bryan Bulten (bbulten@telus.net)
# (C) 2003 Bryan Bulten
#
# See the file "license.txt" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

#
# To build, place source tarball in correct location 
# (i.e. /usr/src/redhat/SOURCES) and run 'rpmbuild -ba musik.spec'.
#
# RPMs should be build in RPM build location 
# (i.e. /usr/src/redhat/RPMS and SRPMS)
#

Summary: Fast, efficient multimedia library program.
Name: Musik
Version: 0.1.2
Release: 1
License: GPL
Group: Applications/Multimedia

#Source: http://musik.sourceforge.net/
Source: Musik-0.1.2.tar.gz

URL: http://musik.sourceforge.net/
BuildRoot: /var/tmp/%{name}-root
Packager: Bryan Bulten <bbulten@telus.net>
Prefix: /usr/local

%description
Fast, efficient multimedia library (program, not development library),
currently supporting mp3 and ogg formats, and sporting a real SQL database with
powerful batch tagging features.

%prep
%setup -q

%build
cmake
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin

make install

%clean
rm -rf $RPM_BUILD_ROOT

#%post 

#%files 
#%defattr(-,root,root)

%changelog
* Mon Jul 10 2003 Bryan Bulten <bbulten@telus.net>
Initial RPM release.

