#! /bin/sh

# report a full platform name, such as slf6-x86_64
# accepts a single argument, which is either empty or "NULL"

default_osname()
{
      OSnum1=`uname -r | cut -f1 -d"."`
      OSnum2=`uname -r | cut -f2 -d"."`
      osname=${current_os}${OSnum1}${OSnum2}
}

check_further ()
{
  if [ -e /usr/bin/lsb_release ]
  then
     check_with_lsb_release
  elif [ ${current_os} = "Linux" ]
  then
     check_linux
  else
     default_osname
  fi
}

check_with_lsb_release ()
{
   OSnum=`lsb_release -r -s | cut -f1 -d"."`
   if [ "$(lsb_release -d -s | cut  -f1 -d" ")" = "Ubuntu" ]
   then
      osname=u${OSnum}
   elif [ "$(lsb_release -i -s)" = "LinuxMint" ]
   then
      # Mint is a Ubuntu variant, but they go to great pains to hide that
      # and their release numbering does not match the Ubuntu release numbering
      osname=mint${OSnum}
   elif [ "$(lsb_release -i -s)" = "SUSE LINUX" ]
   then
      osname=su${OSnum}
   elif [ "$(lsb_release -i -s)" = "RedHatEnterpriseServer" ]
   then
      osname=rh${OSnum}
   elif [ "$(lsb_release -i -s)" = "ScientificFermi" ]
   then
      osname=slf${OSnum}
   elif [ "$(lsb_release -i -s)" = "Scientific" ]
   then
      osname=sl${OSnum}
   elif [ "$(lsb_release -i -s)" = "CentOS" ]
   then
      osname=c${OSnum}
   elif [ ${current_os} = "Linux" ]
   then
      osname=${current_os}${OSnum}
      # Scientific Linux
      if [ "$(lsb_release -d | cut  -f3 -d" ")" = "SL" ]
      then
	 osname=sl${OSnum}
      # Scientific Linux Fermi
      elif [ "$(lsb_release -d | cut  -f3 -d" ")" = "SLF" ]
      then
	 osname=slf${OSnum}
      # Scientific Linux CERN
      elif [ "$(lsb_release -d | cut  -f4 -d" ")" = "SLC" ]
      then
	 osname=slc${OSnum}
      elif [ "$(lsb_release -d | cut  -f4 -d" ")" = "LTS" ]
      then
	 osname=rh${OSnum}
      else
	 default_osname
      fi
   fi
}

check_linux ()
{
   if [ -e /etc/redhat-release ]
   then
      if [ "$(cat /etc/redhat-release | cut  -f4 -d" ")" = "SLC" ]
      then
         OSnum=`cat /etc/redhat-release | cut -f6 -d" " |  cut -f1 -d"."`
         osname=slc${OSnum}
      elif [ "$(cat /etc/redhat-release | cut  -f3 -d" ")" = "Fermi" ]
      then
         OSnum=`cat /etc/redhat-release | cut -f5 -d" " |  cut -f1 -d"."`
         osname=sl${OSnum}
      elif [ "$(cat /etc/redhat-release | cut  -f3 -d" ")" = "SLF" ]
      then
         OSnum=`cat /etc/redhat-release | cut -f5 -d" " |  cut -f1 -d"."`
         osname=slf${OSnum}
      else
        default_names
      fi
   else
      default_names
   fi
}

centos_or_sl7()
{
  osname=unknown
  if [ -e /usr/bin/lsb_release ]
  then
    if [ "$(/usr/bin/lsb_release -i -s)" = "Scientific" ]
    then
        osname=sl7
    elif [ "$(/usr/bin/lsb_release -i -s)" = "CentOS" ]
    then
        osname=c7
    fi
  elif [ -e /etc/system-release-cpe ]
  then
     if [ "$(cat /etc/system-release-cpe | cut  -f3 -d":")" = "redhat" ]
     then
        osname=sl7
     elif [ "$(cat /etc/system-release-cpe | cut  -f3 -d":")" = "centos" ]
     then
        osname=c7
     fi
  elif [ -e /etc/redhat-release ]
  then
     if [ "$(cat /etc/redhat-release | cut  -f1 -d" ")" = "Scientific" ]
     then
        osname=sl7
     elif [ "$(cat /etc/redhat-release | cut  -f1 -d" ")" = "CentOS" ]
     then
        osname=c7
     fi
  fi
}

## execution starts here

null_flavor=${1}

# special case for null flavor
if [ "${null_flavor}" = "NULL" ]
then 
  echo "noarch"
  exit 0
fi

# special case for Linuxppc
flvr1=`ups flavor -1`
if [ "${flvr1}" = "Linuxppc" ]
then
  echo "slf5-ppc"
  exit 0
fi

current_os=`uname`
plat=`uname -m`
osname=undefined

# Darwin is simple
if [ "${current_os}" = Darwin ]
then
  osname=d$(uname -r | cut -f1 -d".")
  echo ${osname}-${plat}
  exit 0
fi

# everything else
flvr=`ups flavor`

case ${flvr} in
  "Linux64bit+2.6-2.12" )
    # historically, do not differentiate
    osname=slf6
    ;;
  "Linux64bit+2.6-2.5" )
    # historically, do not differentiate
    osname=slf5
    ;;
  "Linux64bit+3.10-2.17" )
    centos_or_sl7
    ;;
  "Linux64bit+3.13-2.19" )
    osname=u14
    ;;
  "Linux64bit+3.16-2.19" )
    osname=u14
    ;;
  "Linux64bit+3.19-2.19" )
    osname=u14
    ;;
  * )
    osname=unknown
    ;;
esac

if [ "${osname}" = "unknown" ]
then
  check_further
fi

echo ${osname}-${plat}

exit 0


