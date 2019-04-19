#!/bin/bash -e

#main version is taishanos7, no sp
while getopts 'a:r:v:t:h' OPTION; do
  case $OPTION in
    a)
      ARCH=${OPTARG}
      ;;
    r)
      RELEASE=${OPTARG}
      ;;
    v)
      VERSION=${OPTARG}
      ;;
    t)
      TYPE=${OPTARG}
      ;;
    h)
      echo ""
      echo "Usage: sudo ./iso.sh -v VERSION -r RELEASE -a ARCH"
      echo ""
      echo "Options:"
      echo "  -v VERSION       大版本号，默认为7"
      echo "  -r RELEASE       ServicePack号，默认为0"
      echo "  -a ARCH          架构平台，默认为x86"
      echo "  -t type          s for server, d for desktop"
      echo "  -h               显示本帮助"
      echo ""
      echo "Examples:"
      echo "  sudo ./iso.sh -v 7.0 -r sp5 -t s"
      echo ""
      exit 0
      ;;
  esac
done

BASEDIR=${PWD}
PRODUCT=TaishanOS
VERSION="${VERSION:-7}"
ARCH=${ARCH:-x86_64}
RELEASE="${RELEASE:-0}"
RELEASE=`echo ${RELEASE} | tr [A-Z] [a-z]`
REPO_VER="${VERSION%%.0}.${RELEASE##sp}"
#LABEL=${PRODUCT}-${VERSION}.${RELEASE}.${ARCH}
LABEL=${PRODUCT}-${VERSION}.${ARCH}
#REPOSITORY="http://192.168.0.170/ems-iso/7.4/os/x86_64/"
platform=x86_64
#echo t=$TYPE
if [[ "X$TYPE" == "Xs" ]]; then
	REPOSITORY_os="/res5/"
	#REPOSITORY_updates="/resu/"
	PKG_GROUP=groups_server.xml
	ISOREPO=${BASEDIR}/iso-s.repo
	OUTPUTDIR=${BASEDIR}/iso-s
	PRODUCT_FULLNAME=${PRODUCT}-${VERSION}.${ARCH}-server
elif [[ "X$TYPE" == "Xd" ]]; then
	REPOSITORY_os="/red5/"
	#REPOSITORY_updates="/redu/"
	PKG_GROUP=groups_desktop.xml
	ISOREPO=${BASEDIR}/iso-d.repo
	OUTPUTDIR=${BASEDIR}/iso-d
	PRODUCT_FULLNAME=${PRODUCT}-${VERSION}.${ARCH}-desktop
fi
TMPDIR=${BASEDIR}/cache
#this repo is ISO repo(ts7os, centos_minim...)
#PKG_GROUP=groups_desktop.xml
#YUM_PARAMS="--disablerepo=* --enablerepo=local-epel --enablerepo=iso-* -c $ISOREPO"
YUM_PARAMS="--disablerepo=* --enablerepo=iso-base --enablerepo=iso-extras -c $ISOREPO"
grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u >./pkgs

[[ "0" != "${UID}" ]] && echo "use root!" && exit 1

test -d ${OUTPUTDIR} && rm -rf ${OUTPUTDIR}
rm -rf ${TMPDIR} || true
mkdir -p ${TMPDIR}
yum clean metadata

echo "wait....lorax"

#need install lorax, and lorax need selinux must be disabled or in Permissive mode
#maybe not necessary: need yum install some pkgs: rpm-ostree ...
echo lorax --tmp=${TMPDIR} --isfinal --volid=${LABEL} --buildarch=${ARCH} -p ${PRODUCT} -v ${VERSION} -r ${RELEASE} -s ${REPOSITORY_os} ${OUTPUTDIR}
lorax --tmp=${TMPDIR} --isfinal --volid=${LABEL} --buildarch=${ARCH} -p ${PRODUCT} -v ${VERSION} -r ${RELEASE} -s ${REPOSITORY_os} ${OUTPUTDIR}
SPID=$!

echo "===============================================... yum --releasever=============================="

sleep 3s

yum  $YUM_PARAMS clean metadata

echo "==================wait.... yumdownloader======================"

echo yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` --destdir=${OUTPUTDIR}/Packages 2>&1 >>yumdownloader.log
yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` --destdir=${OUTPUTDIR}/Packages 2>&1 >yumdownloader.log
#yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` `grep "<match install=" ${PKG_GROUP} | sed 's;\(<match install="\)\(.*\)\(%s" name=".*\);\2*;g' | sort -u ` --destdir=${OUTPUTDIR}/Packages 2>&1 >yumdownloader.log
pushd ${OUTPUTDIR}
rm -f Packages/*.i686.rpm
ls Packages/*.rpm > ../pkg.list

echo "===============================wait.... createrepo====================================="

createrepo -g ${BASEDIR}/${PKG_GROUP} -i ../pkg.list .
#rm -f .pkg.list
#wget http://192.168.0.170/ems-iso/7.4/RPM-GPG-KEY-EmindServer-7
#wget https://mirrors.tuna.tsinghua.edu.cn/centos/7.5.1804/os/x86_64/RPM-GPG-KEY-CentOS-7
#use signed key!!!
#cp /home/gavin/iso-tools/RPM-GPG-KEY-CentOS-7-aarch64  ${OUTPUTDIR}/
cp ${BASEDIR}/RPM-GPG-KEY-TaishanOS-7 ${OUTPUTDIR}/

#wget http://mirrors.emindsoft.com.cn/emindserver/${REPO_VER}/RPM-GPG-KEY-${PRODUCT}-release
popd

wait ${SPID}

echo "wait.... efiboot"

if [[ -f "${OUTPUTDIR}/images/efiboot.img" ]]; then
	echo "use efiboot"
	_iso_efi_boot_args="-eltorito-alt-boot
	-e images/efiboot.img
	-no-emul-boot
	-isohybrid-gpt-basdat"
fi
#_iso_efi_boot_args="-r -J -joliet-long -c boot.catalog -efi-boot-part --efi-boot-image -e images/efiboot.img"
#_iso_efi_boot_args="-r -J -joliet-long -c boot.catalog -e images/efiboot.img"
#_iso_efi_boot_args="-r -J -joliet-long -c boot.catalog " ${_iso_efi_boot_args}

rm -f "${OUTPUTDIR}/images/boot.iso" || true

echo "wait.... xorriso"

xorriso -as mkisofs \
        -iso-level 3 \
        -full-iso9660-filenames \
        -volid "${LABEL}" \
        -appid "${PRODUCT} <http://www.tsgxos.com>" \
        -publisher "${PRODUCT} Install CD" \
        -preparer "prepared by mkarchiso" \
        -eltorito-boot isolinux/isolinux.bin \
        -eltorito-catalog isolinux/boot.cat \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        -isohybrid-mbr /usr/share/syslinux/isohdpfx.bin \
        ${_iso_efi_boot_args} \
        -output "${PRODUCT_FULLNAME}.iso" \
        "${OUTPUTDIR}" 
        #-isohybrid-mbr ./null.mbr \
        #-no-emul-boot -boot-load-size 4 -boot-info-table \
#on Phytium: boot-load-size and boot-info-table MUST NOT INVOLVE!
#-efi-boot-part create 3 partitions

implantisomd5 --force --supported-iso "${PRODUCT_FULLNAME}.iso"

md5sum "${PRODUCT_FULLNAME}.iso" > "${PRODUCT_FULLNAME}.iso.md5sum"

sync
sleep 2s
rm -rf ${TMPDIR}

