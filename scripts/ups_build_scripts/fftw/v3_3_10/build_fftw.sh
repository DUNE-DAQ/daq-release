#!/bin/bash
########################################################################
# build_fftw.sh
#
# Build fftw and package for UPS.
#
# Adapted from build.sh.template provided by ssibuildshims v2_09_00.
########################################################################

########################################################################
# Non-customize-able boilerplate: skip to "Customize-able functions."
########################################################################

####################################
# Useful variables.
prog=${BASH_SOURCE[0]##*/}

##################
# Local (dumb) ssi_die() function for use until we source ssi_functions.
ssi_die() {
  printf "FATAL ERROR: ${*}\n" 1>&2; exit 1
}
##################

##################
# Basic usage function for use by ensure_buildshims until we have enough
# information to provide something more specific. Do NOT customize!
build_script_usage() {
    cat <<EOF
USAGE: ${prog} <ssi-msg-config-options> [--] <product-dir> <qualifier-args> <maketar>
       ${prog} -h|--help [<product-dir>]

  Build ${package} and package for UPS.

OPTIONS

  -h|--help

    Help. Supply <product-dir> for specific, detailed information on
    invoking ${prog}.

  <ssi-msg-config-options>

    Supply <product-dir> for more information.

ARGUMENTS

  <product-dir>

    The top-level UPS directory in which the source code distribution
    was unpacked. When specified with -h|--help, this enables us to
    provide more specific help about invoking ${prog}.

  <qualifier-args>
  <maketar>

    Supply <product-dir> for more information.

EOF
}
##################

##################
# Make sure we have UPS and ssibuildshims set up.
ensure_ssibuildshims() {
  if ! (( $# )) || [ "${@: -1}" = "-h" ] || [ "${@: -1}" = "--help" ]; then
    build_script_usage
    exit 1
  fi
  # Find <product-dir> from our arguments:
  local arg
  for arg in "${@}"; do
    [[ "$arg" ==  -* ]] && continue;
    product_dir="${arg}"; break
  done
  if [ -z "${product_dir}" ]; then
    # Bad usage.
    ssi_die "Bad <product-dir> ${product_dir}\n$(build_script_usage)"
    exit 2
  elif ! { [ -d "${product_dir}/.upsfiles" ] && [ -w "${product_dir}" ]; }; then
    # Something wrong with <product-dir>.
    ssi_die "<product-dir> not a valid, writable unified UPS directory: " \
      "${product_dir}\n$(build_script_usage)"
  fi
  # Promising...
  local abs_product_dir="$(cd "${product_dir}" 2>/dev/null 2>&1 && pwd -P)"
  (( $? )) && ssi_die "unable to cd to <product-dir>: ${product_dir}\n" \
    "$(build_script_usage)"
  # Find our own bootstraps and yank 'em.
  local ssibuildshims_topdir="${product_dir}/ssibuildshims/${ssibuildshims_version}"
  [ -f "${ssibuildshims_topdir}/bin/ssi_functions" ] && \
    source "${ssibuildshims_topdir}/bin/ssi_functions" >/dev/null 2>&1 && \
    ensure_ups "${product_dir}" && \
    setup ssibuildshims ${ssibuildshims_version} -z "${product_dir}" || \
    ssi_die "unable to find ssibuildshims ${ssibuildshims_version} in
${product_dir} and set it up with UPS."
  # Absolute is safer.
  absolute_path -- "${product_dir}" product_dir
}
##################

########################################################################
# Customize-able functions.
########################################################################

##################
# Ascertain whether we built and installed correctly. Add more /
# different calls to verify_in_filesystem as appropriate. Arguments
# (e.g. -q) are passed through to verify_in_filesystem.
is_built() { # *C*
  ${trace} verify_in_filesystem -d "${@}" "${pkgdir}/lib"{64,}
}
##################

##################
# If a distribution archive is wanted and the installation is successful
# according to is_built(), then make it. Any arguments (e.g. -q) are
# passed through both to is_built() and make_basic_distribution().
#
# If make_basic_distribution() finds that the product is declared
# correctly and attempts to make the distribution, then it will cause us
# to exit with the status code of that operation. If not, it returns an
# error code.
#
# See make_basic_distribution -h for useful options to add to
# mbd_opts(). *C*
maybe_make_distribution() {
  if [ "${maketar}" = "tar" ]; then
    # Customize mbd_opts. *C*
    local mbd_opts=()
    ${trace} is_built "$@" && \
      ${trace} make_basic_distribution "${@}" "${mbd_opts[@]}"
  fi
}
##################

########################################################################
# Package-specific functions.
########################################################################

# ...

########################################################################
# Main.
########################################################################

####################################
# Useful variables. *C*
trace=maybe_xtrace # Could set to xtrace instead for unconditional.
(( ssi_trace = 1 )) # Trace certain commands by default.
####################################

####################################
# Package name and version information. *C*
package=fftw
origpkgver=v3_3_10
pkgver=${origpkgver} # Append letters for local patches.
ssibuildshims_version=v2_15_00
pkgdotver=${origpkgver//_/.}; pkgdotver=${pkgdotver#v}
####################################

##################
# Other useful variables. *C*
pkgtarname=${package}-${pkgdotver}
pkgtarfile=${pkgtarname}.tar.gz
##################

####################################
# Handle options and arguments.

# Set up ssibuildshims and UPS / product_dir as early as possible.
ensure_ssibuildshims "${@}"

##################
# Now parse options and arguments, setting basequal, extraqual, maketar,
# etc.
#
# Use verify_quals() to parse command line options, verify that the user
# has provided qualifiers supported by this build script, and set
# expected variables such as basequal, extraqual, cqual, etc.
#
# Customize the extra usage text as appropriate. Note that DEBUG_PATCHES
# and VERBOSE_PATCHES are honored by prepare_source() and
# patch_source(). DEBUG_(CONFIG|BUILD|TEST|INSTALL) should be accounted
# for (or not) by the relevant command in this script.
#
# See verify_quals -h for options and examples.
verify_quals \
  --parse --usage-file=- -- "$@" <<EOF || exit # *C*
ENVIRONMENT

  DEBUG_(BUILD|TEST|INSTALL)

    Verbose output from the corresponding step.

  DEBUG_PATCHES
  VERBOSE_PATCHES

    Command tracing and increased verbosity for source unpacking and patching
    operations. DEBUG_PATCHES will cause exit after patches have been applied.

  FFTW_NO_CHECKS
  NO_TESTS

    Disable post-build checks.
EOF
##################

##################
# Define globals: pkgdir, sourcedir, blddir, fullqual, etc. See
# define_basics -h for steering options.
${trace} define_basics NO_QUAL TRACK_PROGRESS || exit # *C*
##################

####################################
# Declare ourselves privately, and then set ourselves up to allow setup
# of dependencies. *C*
#
# Comment or delete if not required:
${trace} fake_declare_product "${ssi_args[@]}" -x -- || exit
####################################

####################################
# Build-only dependencies. *C*
#
# e.g.
#   ${trace} setup_cmake --latest={--ge,3.15} || exit
#      (version requirement necessary for cmake --install).
# AND/OR
#   ${trace} setup_ninja --optional && (( want_ninja = 1 ))
####################################

# Print active products.
echo
ups active
echo

# Possible short-circuit if we're already built and declared.
${trace} maybe_make_distribution --silent

ssi_info "building ${package} for ${OS}-${plat}-${qualdir} (flavor ${pkgflvr})"

# Ensure directories: pkgdir, sourcedir, blddir.
${trace} init_pkg_dirs || exit

####################################
# Prepare source for building. *C*

${trace} cd "${sourcedir}" \
  || ssi_die "unable to cd to source directory ${sourcedir}"

# Source tarballs and patches. See prepare_source -h / patch_source -h
# for options.
${trace} prepare_source "${ssi_args[@]}" -P "${patchdir}" \
  "${tardir}/${pkgtarfile}" \
  || exit

# Bail early if DEBUG_PATCHES is set.
[ -n "${DEBUG_PATCHES}" ] && exit 1
####################################

####################################
# Prepare to build. *C*
#
# Set configure / compile flags, environment variables, etc.

configure_opts=(--enable-shared
  --disable-fortran
  --enable-openmp
  --enable-threads
  --enable-fma
)
# Options only valid at lower precisions
configure_opts_sd=(--enable-{sse2,avx,avx2})

ccflg=(-O3 -fno-math-errno -DNDEBUG)

[ "$OS1" = Darwin ] && cc=clang || (( want_quad = 1 ))
if { [[ "$OS" =~ ^u([[:digit:]]+) ]] && (( ${BASH_REMATCH[1]} >= 16 )); } || \
  [[ "${OS1}" = Darwin ]]; then
  configure_opts_sd+=(--enable-avx512 --enable-avx-128-fma)
fi

##################
# Configure parallelism. *C*
ncores=$(ncores)
####################################

####################################
# Configure, build, and install.
#
# Use ${blddir}/${pkgtarname} for in-source builds, ${blddir} otherwise.

${trace} cd "${blddir}" || \
  ssi_die "unable to cd to build directory ${blddir}"

##################
# Configure.
# Build.
# Test.
# Install.

make_and_install() {
  local extra_opts=("$@")
  ssi_info "building FFTW with ${extra_opts[*]:-default options}"
  [ -n "$blddir" ] && cd "$blddir" && rm -rf *
  ${trace} env ${cc:+CC=${cc}} CFLAGS=${ccflg[*]:+"${ccflg[*]}"} \
    ${sourcedir}/${pkgtarname}/configure --prefix="${pkgdir}" \
    "${configure_opts[@]}" "${extra_opts[@]}" || \
    ssi_die "configure failed with ${extra_opts[*]:-default options}"
  ${trace} make -j ${ncore} ${DEBUG_BUILD:+V=1} || \
    ssi_die "build failed with ${extra_opts[*]:-default options}"
  (( FFTW_NO_CHECKS || NO_TESTS )) || \
    ${trace} make -j ${ncore} check ${DEBUG_TEST:+V=1} || \
    ssi_die "tests failed with ${extra_opts[*]:-default options}"
  ${trace} make -j ${ncore} install ${DEBUG_INSTALL:+V=1} || \
    ssi_die "install failed with ${extra_opts[*]:-default options}"
}

${trace} make_and_install --enable-long-double
${trace} make_and_install "${configure_opts_sd[@]}"
${trace} make_and_install "${configure_opts_sd[@]}" --enable-single
(( want_quad )) && ${trace} make_and_install --enable-quad-precision

ssi_info "finished building ${package} ${pkgver}"

####################################
# Pre-declaration processing. *C*
#
# Move things, install from contrib, clean build products from in-source
# builds, etc.
####################################

# Note that we don't use CMake to build because it doesn't have the
# range of microarchitectures the configure build does (as at
# 3.3.10). What are installed by way of CMake config files are broken,
# so must be removed.
${trace} rm -rf "${pkgdir}/lib/cmake" && \
  ${trace} fix_pc && ${trace} smoke_test_pc || \
    ssi_die "post-processing failed"

${trace} cp -r ${product_dir}/${package}/${pkgver}/cmake ${pkgdir}/lib

##################
# Declare and set up the installed package. *C*
#
# Add -c for current declaration if required.
${trace} declare_product -x -- || exit
##################

####################################
# Post-declaration processing. *C*
#
# Operations requiring the *final* declaration and setup (as opposed to
# the pre-build fake declaration and setup). This should be rare.
####################################

#################
# Verify installation and make distribution archive. *C*
#
# To customize verification, see is_built(), since it is also used by
# maybe_make_distribution(). See the documentation for those functions
# (above) for customization opportunities.

# Add a message to ssi_build_failed if appropriate.
${trace} is_built && ${trace} ssi_build_completed && \
  ssi_info "${package} is installed at ${pkgdir}" && \
  ${trace} maybe_make_distribution || \
  ssi_build_failed # *C*
########################################################################
# Nothing beyond this point.
########################################################################
