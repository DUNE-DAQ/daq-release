function build_fftw() {
  prep_build fftw ${1} || return 0
  NO_TESTS=1 ./build_fftw.sh ${product_topdir} ${maketar} >& "${logfile}"
}

# Local Variables:
# mode: sh
# eval: (sh-set-shell "bash")
# End:
