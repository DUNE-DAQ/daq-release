#!/bin/bash

RUN_ID=$1
REPO="DUNE-DAQ/daq-release"

WORKFLOW_DETAILS=$(gh api /repos/$REPO/actions/runs/${RUN_ID})

WORKFLOW_NAME=$(echo "$WORKFLOW_DETAILS" | jq -r '.name')
ACTOR_LOGIN=$(echo "$WORKFLOW_DETAILS" | jq -r '.actor.login')
EVENT_TYPE=$(echo "$WORKFLOW_DETAILS" | jq -r '.event')
HTML_URL=$(echo "$WORKFLOW_DETAILS" | jq -r '.html_url')

gh api /repos/$REPO/actions/runs/${RUN_ID}/jobs \
    | jq --arg workflow   "$WORKFLOW_NAME"   \
         --arg actor      "$ACTOR_LOGIN"     \
         --arg event      "$EVENT_TYPE"      \
         --arg html_url   "$HTML_URL" '      
def job_summary(status):
  [.jobs[]
   | select(.conclusion == status)
   | {
       job: .name,
       conclusion: .conclusion,
       steps: [
         .steps[]
         | select(.conclusion == status)
         | { name: .name }
       ]
     }
  ];

{
  workflow:   $workflow,
  actor:      $actor,
  event:      $event,
  html_url:   $html_url,
  failed_jobs:    job_summary("failure"),
  skipped_jobs:   job_summary("skipped"),
  cancelled_jobs: job_summary("cancelled")
}
' | tee workflow_summary.json
