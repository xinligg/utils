#!/bin/bash -e

#main version Superredos 7
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
      echo "Usage: sudo ./mkccos.sh -v VERSION -r RELEASE -a ARCH"
      echo ""
      echo "Options:"
      echo "  -v VERSION       大版本号，默认为7.0"
      echo "  -r RELEASE       ServicePack号，默认为0"
      echo "  -a ARCH          架构平台，默认为x86"
      echo "  -t type          s for server, d for desktop"
      echo "  -h               显示本帮助"
      echo ""
      echo "Examples:"
      echo "  sudo ./mkccos.sh -v 7.7 -r 1910 -a x86_64"
      echo ""
      exit 0
      ;;
  esac
done

#base info
BASEDIR=${PWD}
PRODUCT=CCOS
VERSION="${VERSION:-7.0}"
ARCH=${ARCH:-x86_64}
RELEASE="${RELEASE:-0}"
RELEASE=`echo ${RELEASE} | tr [A-Z] [a-z]`
LABEL=${PRODUCT}-${VERSION%%.*}.${ARCH}
#LABEL=${PRODUCT}-${VERSION}.${RELEASE}.${ARCH}

#repo url
REPO_VER="${VERSION}.${RELEASE##sp}"
#REPO_URL="http://repo.superred.com.cn"
REPO_URL="http://10.10.3.101"

if [[ $ARCH == "x86_64" ]]; then
	REPO_FLAG="superredos"
else
	REPO_FLAG="altarch"
fi

KERNEL_VERSION="kernel-4.19-ft-server"
REPOSITORY="${REPO_URL}/${REPO_FLAG}/${REPO_VER}/os/${ARCH}"
REPOSITORY_updates="${REPO_URL}/${REPO_FLAG}/${REPO_VER}/updates/${ARCH}"
REPOSITORY_kernel="${REPO_URL}/${REPO_FLAG}/${REPO_VER}/kernel/${ARCH}/${KERNEL_VERSION}"
REPOSITORY_localrepo="file:///home/superredos/work/mkiso/s77/oem/localrepo"

echo REPOSITORY=$REPOSITORY
echo REPOSITORY_updates=$REPOSITORY_updates
echo REPOSITORY_kernel=$REPOSITORY_kernel
echo REPOSITORY_localrepo=$REPOSITORY_localrepo

DATETIME=`date +%Y%m%d-%H%M`
COMPS_FILE=c${VERSION%%.*}-${RELEASE}-${ARCH}-comps.xml

#repo comps file
if [[ ! -f "${BASEDIR}/$COMPS_FILE" ]]; then
	echo "repo comps file '${COMPS_FILE}' is not exist!"
	exit 0
	#echo "create repo comps file..."
    	#createrepo_c -g ${BASEDIR}/${COMPS_FILE} --update /data/webroot/superredos/${REPO_VER}/os/x86_64/
else
	echo "update repo comps file..."
    	#createrepo_c -g ${BASEDIR}/${COMPS_FILE} --update /data/webroot/superredos/${REPO_VER}/os/x86_64/
fi

#product info
PKG_GROUP=${COMPS_FILE}
ISOREPO=${BASEDIR}/CCOS-${VERSION%%.*}-${RELEASE}-${ARCH}.repo
OUTPUTDIR=${BASEDIR}/iso-$ARCH
PRODUCT_FULLNAME=${PRODUCT}-${VERSION%%.*}-${ARCH}-${RELEASE}-${DATETIME}

#yum repo enable
TMPDIR=${BASEDIR}/cache
#this repo is ISO repo(yd7os, centos_minim...)
YUM_PARAMS="--disablerepo=* --enablerepo=base --enablerepo=updates --enablerepo=epel --enablerepo=${KERNEL_VERSION} --enablerepo=localrepo -c $ISOREPO"
grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u >./pkgs

[[ "0" != "${UID}" ]] && echo "use root!" && exit 1

test -d ${OUTPUTDIR} && rm -rf ${OUTPUTDIR}
rm -rf ${TMPDIR} || true
mkdir -p ${TMPDIR}
yum clean metadata

echo "wait....lorax"

setenforce 0
#need install lorax, and lorax need selinux must be disabled or in Permissive mode
#maybe not necessary: need yum install some pkgs: rpm-ostree ...
echo lorax --tmp=${TMPDIR} --isfinal --volid=${LABEL} --buildarch=${ARCH} -p ${PRODUCT} -v ${VERSION} -r ${RELEASE} -s ${REPOSITORY} -s ${REPOSITORY_updates} -s ${REPOSITORY_kernel} -s ${REPOSITORY_localrepo} ${OUTPUTDIR}
lorax --tmp=${TMPDIR} --isfinal --volid=${LABEL} --buildarch=${ARCH} -p ${PRODUCT} -v ${VERSION} -r ${RELEASE} -s ${REPOSITORY} -s ${REPOSITORY_updates} -s ${REPOSITORY_kernel} -s ${REPOSITORY_localrepo} ${OUTPUTDIR}
SPID=$!

setenforce 1
echo "===============================================... yum --releasever=============================="

sleep 3s

yum  $YUM_PARAMS clean metadata

echo "==================wait.... yumdownloader======================"

echo yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` --destdir=${OUTPUTDIR}/Packages 2>&1 >>yumdownloader.log
yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` --destdir=${OUTPUTDIR}/Packages 2>&1 >>yumdownloader.log
#yumdownloader --archlist=${ARCH} $YUM_PARAMS --installroot=${TMPDIR} --resolve `grep "<packagereq" ${PKG_GROUP} | sed 's;\(<packagereq.*\)>\(.*\)\(</packagereq>.*\);\2;g' | sort -u` `grep "<match install=" ${PKG_GROUP} | sed 's;\(<match install="\)\(.*\)\(%s" name=".*\);\2*;g' | sort -u ` --destdir=${OUTPUTDIR}/Packages 2>&1 >yumdownloader.log
pushd ${OUTPUTDIR}
rm -f Packages/*.i686.rpm
ls Packages/*.rpm > ../pkg.list

echo "===============================wait.... createrepo====================================="

createrepo -g ${BASEDIR}/${PKG_GROUP} -i ../pkg.list .
#rm -f .pkg.list
#use signed key!!!
curl $REPO_URL/$REPO_FLAG/$REPO_VER/os/$ARCH/RPM-GPG-KEY-SuperRedOS-7 > ${OUTPUTDIR}/RPM-GPG-KEY-SuperRedOS-7

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

#generate ISO
echo "wait.... xorriso"
if [[ $ARCH == "x86_64" ]]; then
	xorriso -as mkisofs \
        	-iso-level 3 \
        	-full-iso9660-filenames \
        	-volid "${LABEL}" \
        	-appid "${PRODUCT} <http://www.ccos.com.cn>" \
        	-publisher "${PRODUCT} Install CD" \
        	-preparer "prepared by mkarchiso" \
        	-eltorito-boot isolinux/isolinux.bin \
        	-eltorito-catalog isolinux/boot.cat \
        	-no-emul-boot -boot-load-size 4 -boot-info-table \
        	-isohybrid-mbr /usr/share/syslinux/isohdpfx.bin \
        	${_iso_efi_boot_args} \
        	-output "${PRODUCT_FULLNAME}.iso" \
        	"${OUTPUTDIR}"
elif [[ $ARCH == "aarch64" ]]; then
	xorriso -as mkisofs \
        	-iso-level 3 \
        	-full-iso9660-filenames \
        	-volid "${LABEL}" \
        	-appid "${PRODUCT} <http://www.ccos.com.cn>" \
        	-publisher "${PRODUCT} Install CD" \
        	-preparer "prepared by mkarchiso" \
        	-no-emul-boot \
        	${_iso_efi_boot_args} \
        	-output "${PRODUCT_FULLNAME}.iso" \
        	"${OUTPUTDIR}"
else
	xorriso -as mkisofs \
        	-iso-level 3 \
        	-full-iso9660-filenames \
        	-volid "${LABEL}" \
        	-appid "${PRODUCT} <http://www.ccos.com.cn>" \
        	-publisher "${PRODUCT} Install CD" \
        	-preparer "prepared by mkarchiso" \
        	-no-emul-boot -boot-load-size 4 -boot-info-table \
        	${_iso_efi_boot_args} \
        	-output "${PRODUCT_FULLNAME}.iso" \
        	"${OUTPUTDIR}"
fi
#on Phytium: boot-load-size and boot-info-table MUST NOT INVOLVE!
#-efi-boot-part create 3 partitions

implantisomd5 --force --supported-iso "${PRODUCT_FULLNAME}.iso"

md5sum "${PRODUCT_FULLNAME}.iso" > "${PRODUCT_FULLNAME}.iso.md5sum"

sync
sleep 2s
rm -rf ${TMPDIR}


# check iso dependencies
LOCALREPO=local.repo
echo -e "[localrepo]" > ./$LOCALREPO
echo -e "name=Local repo for SuperRedOS 7 ISO - $ARCH" >> ./$LOCALREPO
echo -e "baseurl=file://${OUTPUTDIR}/" >> ./$LOCALREPO
echo -e "failovermethod=priority" >> ./$LOCALREPO
echo -e "enabled=1" >> ./$LOCALREPO

repoclosure -r localrepo -c ./$LOCALREPO
