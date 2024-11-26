
success_message="Script was successful"

echo "Check the return value of this script and/or whether you see \"$success_message\" when it finishes"

if [[ -n $( ls -A ) ]]; then

   echo "Error: you need to source his script in an empty directory; returning..." >&2
   return 1
fi

if [[ -d $HOME/.local ]]; then

   echo "WARNING: the presence of a $HOME/.local directory may interfere with the execution of this script" >&2
fi

. /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v2.2/spack-0.22.0/share/spack/setup-env.sh || return 2
spack load python@3.10.10 || return 3

python -m venv freshenv || return 4
. ./freshenv/bin/activate || return 5

# Upgrade pip and install the poetry dependency-management package as
# described in https://python-poetry.org/docs/

pip install --upgrade pip || return 6
pip install pipx || return 7
pipx install poetry || return 8

echo "MAY MODIFY \$PATH in $HOME/.bashrc" >&2
pipx ensurepath || return 9

# Assumption is this script is being sourced (not executed) and that
# pyproject.toml exists in the same directory as the script

scriptdir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cp $scriptdir/pyproject.toml . || return 10

# Unclear if this is actually "shielding" you from the surrounding environment
# Create a new subdirectory and cd into it before calling this?

poetry shell || return 11
poetry install || return 12

pip freeze > ./pyvenv_requirements.txt
test "$?" == "0" || return 12

echo "Versions frozen into ./pyvenv_requirements.txt"
echo $success_message

