function build_protobuf() {
  prep_build protobuf ${1} ${2} || return 0
  ./build_protobuf.sh ${product_topdir} ${2}  ${maketar} >& "${logfile}"
}

# Local Variables:
# mode: sh
# eval: (sh-set-shell "bash")
# End:
