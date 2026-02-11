# Building Different DAQ Environments

## Overview

This document describes the work done to support building different DAQ environments independently, addressing the need for more flexible and modular release workflows. This work was primarily implemented in the `johnfreeman/issue500_different_environments` branch and has since been merged into the main development branch.

## Background

The DUNE DAQ software stack is organized into multiple detector-specific environments:

- **coredaq**: Core DAQ functionality used across all detector configurations
- **fddaq**: Far Detector DAQ packages (includes coredaq + FD-specific packages)
- **nddaq**: Near Detector DAQ packages (includes coredaq + ND-specific packages)

Previously, building any detector-specific environment required triggering a full release build, which was resource-intensive and time-consuming. There was a need to build and test specific environments independently without affecting the main release pipeline.

## What Was Implemented

### New Workflow: build-fddatautilities-alma9.yml

The primary addition was a new GitHub Actions workflow that enables independent building of the `fddatautilities` package and related Far Detector utilities on AlmaLinux 9.

**Location**: `.github/workflows/build-fddatautilities-alma9.yml`

This workflow provides:

1. **Manual Trigger Capability** (`workflow_dispatch`): Build on-demand without waiting for nightly schedules
2. **Flexible Tag Management**: Custom tag prefixes for different build purposes
3. **Feature Branch Support**: Test builds from any development branch
4. **Optional CVMFS Deployment**: Deploy builds to `/cvmfs` when ready

### Workflow Inputs

The workflow accepts three configurable inputs:

```yaml
tag-prefix:
  description: 'fddatautilities tag prefix'
  default: ''
  
feature-branch:
  description: 'feature branch to be used across all DAQ repos wherever possible'
  default: develop
  
cvmfs-deployment:
  description: 'whether to deploy the release to cvmfs'
  default: 'no'
```

### Tag Format

The workflow creates nightly-style tags using the format:
```
{tag-prefix}_DEV_{YYMMDD}_A9
```

Where:
- `{tag-prefix}`: User-provided prefix (e.g., "fddatautilities")
- `_DEV_`: Indicates development/nightly build
- `{YYMMDD}`: Date stamp
- `_A9`: Indicates AlmaLinux 9 build

Example: `fddatautilities_DEV_240412_A9`

## Use Cases

### 1. Testing FD-Specific Changes

When making changes that only affect Far Detector packages, developers can now:
- Trigger a build specifically for `fddatautilities`
- Test against a feature branch
- Avoid triggering full `fddaq` release builds

### 2. Independent Package Development

Teams working on detector-specific utilities can:
- Build and deploy their packages independently
- Maintain separate release cadences
- Reduce CI/CD resource usage

### 3. Rapid Iteration and Testing

For development and debugging:
- Quick builds without full release overhead
- Test specific configurations
- Validate changes before merging to main release

## How to Use

### Triggering a Build

1. Navigate to the [Actions page](https://github.com/DUNE-DAQ/daq-release/actions)
2. Select "Alma9 fddatautilities build" from the workflow list
3. Click "Run workflow"
4. Configure the inputs:
   - **tag-prefix**: Enter desired tag prefix (e.g., "fddatautilities_test")
   - **feature-branch**: Select branch to build (default: `develop`)
   - **cvmfs-deployment**: Choose "yes" to deploy to CVMFS (typically "no" for test builds)
5. Click "Run workflow" to start the build

### Accessing Build Artifacts

- Build artifacts are uploaded to GitHub Actions artifacts storage
- Successful builds are available for download from the workflow run page
- If CVMFS deployment is enabled, the build will be available at:
  `/cvmfs/dunedaq-development.opensciencegrid.org/`

## Architecture and Design

### Consistency with Existing Workflows

The `build-fddatautilities-alma9.yml` workflow follows the same pattern as:
- `build-nightly-release-alma9.yml`: Nightly development builds
- `build-candidate-release-alma9.yml`: Release candidate builds
- `build-stable-release-alma9.yml`: Stable production releases

This consistency ensures:
- Maintainable workflow definitions
- Predictable behavior across build types
- Easy onboarding for developers familiar with existing workflows

### Relationship to Spack Build System

The workflow utilizes the Spack-based build system through:
- Build scripts in `scripts/spack/build-release.sh`
- Configuration files in `configs/fddaq/` directory
- Spack environment definitions in `spack-repos/fddaq-repo-template/`

### Docker Images and Externals

Builds execute inside Docker containers that include pre-built external dependencies:
- Base image: `ghcr.io/dune-daq/alma9-slim-externals:v2.X`
- Contains: ROOT, Boost, Python packages, and other external dependencies
- Reduces build time by avoiding external package compilation

## Benefits

1. **Reduced Build Times**: Targeted builds complete faster than full releases
2. **Resource Efficiency**: Conserves GitHub Actions minutes and compute resources
3. **Development Flexibility**: Enables rapid iteration on detector-specific code
4. **CI/CD Independence**: Multiple environments can evolve at different paces
5. **Testing Capability**: Safe testing environment without affecting main releases

## Future Enhancements

Potential extensions of this work could include:

- Additional workflows for `nddatautilities` (Near Detector utilities)
- Support for additional operating systems (e.g., Ubuntu, other RHEL variants)
- Integration with automated testing pipelines
- Enhanced artifact management and versioning
- Cross-environment dependency management

## Related Documentation

- [Nightly Releases and Continuous Integration](ci_github_action.md)
- [Creating a new DAQ release](create_release_spack.md)
- [How to publish files to cvmfs](publish_to_cvmfs.md)
- [DAQ software development workflow](development_workflow_gitflow.md)

## Technical Details

### Workflow Structure

The workflow consists of a single job that:
1. Creates a dated nightly tag with the specified prefix
2. Sets up the build environment using the specified Docker image
3. Checks out the `daq-release` repository
4. Runs the build script with appropriate configurations
5. Uploads build artifacts to GitHub
6. Optionally deploys to CVMFS if requested

### Configuration Management

Environment-specific configurations are stored in:
- `configs/fddaq/`: Far Detector package configurations
- `spack-repos/fddaq-repo-template/`: Spack recipe templates

These configurations specify:
- Package versions and dependencies
- Build variants and options
- Environment setup requirements

## Troubleshooting

### Common Issues

1. **Build Failures**: Check the workflow logs in the GitHub Actions interface
2. **Missing Dependencies**: Verify the externals image version matches requirements
3. **CVMFS Deployment Issues**: Ensure proper credentials and permissions are configured
4. **Feature Branch Not Found**: Verify the branch exists in all required repositories

### Getting Help

- Check the [DUNE DAQ GitHub Issues](https://github.com/DUNE-DAQ/daq-release/issues)
- Contact the Software Coordination team
- Review CI dashboard at [DUNE DAQ CI Summary Dashboard](https://dune-daq.github.io/daq-release/)

## Credits

This work was implemented by John Freeman (JCF) as part of addressing Issue #500, which aimed to provide more flexible and efficient build capabilities for different detector environments within the DUNE DAQ software ecosystem.
