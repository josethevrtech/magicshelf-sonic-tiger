# Publication checklist

1. Review RELEASE_REVIEW.md, LISTING.md, screenshots, CHECKS.json and the release folder. Decide whether to finish individual headset controls and shake/snap checks before publication.
2. Create `josethevrtech/magicshelf-sonic-tiger`, publish the reviewed source on main, and tag the exact release source `v0.2.3`. The repository has been created. Keep local/, vendor/, build/, dist/, saves and supplied originals out of Git. Remove the preparation-only banner in README when actually publishing.
3. Create a draft GitHub release for v0.2.3 with RELEASE_NOTES.md as its body. Upload only the curated files from dist/publish-v0.2.3/. Include the matching source archive, full notices and three screenshot assets as well as the creation package/catalog.
4. Publish the creation release after checking the draft files. Re-download the package/catalog/screenshots from their public immutable URLs and verify checksums, PNG dimensions, credits links and clean extraction. Do not overwrite a version with different bytes.
5. Refresh the MagicShelf default branch and compare it with framework-base/provenance.json. Apply or rebase magicshelf-catalog.patch onto a new codex/ branch. The patch adds the release metadata plus its stable latest catalog to collections.json. Validate both contracts, open a PR and attach the PR to this chat. No framework code or version bump is needed just to list this creation.
6. After the catalog change is approved/merged, refresh MagicShelf and check the actual listing, images, credits, permission disclosure, free install, On/Off and source-owned subscription. Do not restore old creation data or replace other installed modules.

## Authentication note

The currently checked GitHub CLI account is josethevrtech. Its token has repo scope but lacks workflow scope. Pushing workflow files may require an appropriate GitHub connection or a user-controlled authentication update. The workflow was added using the connected GitHub app; the source push then succeeded without changing authentication. The checks workflow builds review artifacts only; it does not automatically publish releases.

## URL targets

- Source/issues: https://github.com/josethevrtech/magicshelf-sonic-tiger
- Release: https://github.com/josethevrtech/magicshelf-sonic-tiger/releases/tag/v0.2.3
- Subscription: https://github.com/josethevrtech/magicshelf-sonic-tiger/releases/latest/download/catalog.json

These are planned publication targets, not claims that the repository or release is already live.
