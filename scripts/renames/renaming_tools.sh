
feature_branch=johnfreeman/daq-release_issue379_renames_pt2

function gitclone {
    for pkg in "$@"; do
	if [[ ! -d $pkg ]]; then
	    git clone https://github.com/DUNE-DAQ/$pkg -b develop
	    if [[ "$?" != "0" ]]; then
		echo "Unable to clone $pkg; exiting.." >&2
		exit 3
	    fi

	    cd $pkg
	    git checkout $feature_branch || git checkout -b $feature_branch
	    cd ..
	fi
    done
}

function replace_token() {

    local old=$1
    local new=$2
    
    if [[ ! -d .git ]]; then
	echo "Looks like $PWD isn't a git repo; exiting..." >&2
	exit 4
    fi

    local tmpdir=$( mktemp -d )
    mv .git $tmpdir
    find . -type f | xargs sed -r -i 's!'$old'!'$new'!g'
    mv $tmpdir/.git .
    rm -rf $tmpdir

}

function fully_replace_token() {
    local old=$1
    local new=$2
    local affected_repos=$3

    gitclone $affected_repos

    for repo in $affected_repos; do
	cd $repo
	replace_token $old $new
	cd ..
    done
}
