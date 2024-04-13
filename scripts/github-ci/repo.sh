
buildtype="develop"
packages_with_ci_single_string=$( ../list_packages.py $buildtype coredaq )" "$( ../list_packages.py $buildtype fddaq )" "$( ../list_packages.py $buildtype nddaq )

read -r -a dune_packages_with_ci <<< $packages_with_ci_single_string

dune_packages=(
  "${dune_packages_with_ci[@]}"
  "connectivityserver"
  "daq-deliverables"
  "daq-docker"
  "docs"
  "nanorc"
  "integrationtest"
  "pocket"
  "elisa_client_api"
  "daq-release"
  "daq-buildtools"
  "daq-assettools"
  "daq-kube"
  "dqm-backend"
  "dqmtools"
  "grafana-dashboards"
  "microservices"
  "datafilter"
  "daqsystemtest"
  "drunc"
  "druncschema"
  "justintime"
  "performancetest"  
  "styleguide"
  "tpgtools"
  "trgtools"
)
