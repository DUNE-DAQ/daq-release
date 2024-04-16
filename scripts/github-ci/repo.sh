
if [[ -z $DEVLINE ]]; then
    echo "ERROR: an environment variable DEVLINE needs to be set for repo.sh to source properly"
    echo "Allowed values are \"develop\" and \"production_v4\""
    echo "It's possible you're running a script which has fallen out of maintenance; please contact John Freeman if you wish to use it"
    return 1
fi

if [[ "$DEVLINE" != "develop" && "$DEVLINE" != "production_v4" ]]; then
    echo "ERROR: environment variable DEVLINE set to an unexpected value \"$DEVLINE\""
    return 2
fi

packages_with_ci_single_string=$( ../list_packages.py $DEVLINE coredaq )" "$( ../list_packages.py $$DEVLINE fddaq )  #" "$( ../list_packages.py $$DEVLINE nddaq )

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
