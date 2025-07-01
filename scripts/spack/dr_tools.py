
import os
import yaml

def parse_yaml_file(fname):
    if not os.path.exists(fname):
        print("Error: -- YAML file {} does not exist".format(fname))
        exit(20)
    fman = ""
    with open(fname, 'r') as stream:
        try:
            fman = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    return fman

def get_packages(yaml_file, package_group):
    rdict = parse_yaml_file(yaml_file)

    pkgs = rdict.get(package_group)

    if pkgs is None:
        return []

    return [pkg["name"] for pkg in pkgs if pkg.get("source", "github_DUNE-DAQ") == "github_DUNE-DAQ"]

