# Release Management

The Release Coordinator helps to define the goals of a release, collects deliverables from the working groups, and helps guide development effort. They also coordinate release acceptance testing and are responsible for delivering a well-tested, stable DAQ release.

A Release Coordinator **DOES**:
* Schedule regular release coordination meetings to ensure that the working groups are making progress towards a release
* Create/Link `daq-deliverables` Issues to track defined deliverables for a release
* Solicits updates from the working groups on deliverable progress
* Reports to the Coordination Team (CT) with progress updates
* Ensures that acceptance tests are run on release candidates

A Release Coordinator **DOES NOT**:
* Define deliverables associated with the release
* Manage working group members directly
* Define acceptance tests for a release

## Timeline

1. Define Goals/Critical Features for Release

    This activity usually happens within the CT; if a Release Coordinator has be identified for the release, they may have input based on ongoing work.

1. Assign Release Coordinator

    This will be done by the DAQ Consortium Leads or one of the Technical Leads (if release is targeted for a specific detector)

1. Collect Deliverables from Working Groups

    The first real task of the Release Coordinator is to build the `daq-deliverables` Issue structure for the release. Each goal/major feature should be represented by a Github Issue, using Sub-Issues (Make a checklist and click "Convert to Issue") is recommended to increase clarity. Smaller tasks within a goal/feature deliverable can be represented as checklist items.

1. Define Target Dates

    General knowledge of the release target should come from the CT, but exact target dates for release candidates are owned by the Release Coordinator, with their overall view of working group progress. There should be **at least** a week allocated between the first release candidate and the release due date for testing.

1. Develop Release

    The primary activity during this period is a set of Release Coordination meetings (~weekly near the beginning of the release, escalating in frequency as needed). These meetings should cover progress on the ongoing development for the release, and any requests to add/reduce scope should be brought at this meeting. The Release Coordinator attends the CT Meetings held during this period to give updates on the release. The "Tag Collector" document should be updated at this time and reflect the latest tags of packages as developments are completed.

1. Cut Release Candidate

    When the Release Coordinator determines that primary development for the release has been completed, they should ask Software Coordination for a release candidate build. The Release Coordinator is reponsible for delivering a completed Tag Collector to Software Coordination so that the candidate release can be created.

1. Perform Acceptance Testing

    Each Working Group should define the acceptance tests relevant to their feature changes, and the Technical Leads will define additional detector-specific tests needed for a release to be accepted. The Release Coordinator should identify developers willing to carry out acceptance tests and track the progress as tests are completed. (In previous releases, a testing spreadsheet was used.) Issues identified during testing should be referred to the appropriate working groups and additional release candidates made as needed.

1. Cut Release

    When the Release Coordinator is satisfied that the release candidate passes the acceptance tests, they should ask Software Coordination to create the final release from the last candidate release.

1. Write Release Notes

    The final job of the Release Coordinator is to create a "Release Notes" document for the release, which should be a high-level account of the new features included in the release. It should link to relevant `daq-deliverable` Issues and serves as the record of what was accomplished for a given release.
