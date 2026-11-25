use std::env;
use std::path::{Path, PathBuf};
use std::process::Command;

fn main() {
    let manifest_dir = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let repo_root = manifest_dir.join("../..");
    let schema = repo_root.join("spec/basic.jaw");
    let output = PathBuf::from(env::var_os("OUT_DIR").unwrap()).join("noodles2.rs");
    let jaw = find_jaw(&repo_root);

    println!("cargo:rerun-if-env-changed=JAW_BIN");
    println!(
        "cargo:rerun-if-changed={}",
        repo_root.join("spec").display()
    );

    let status = Command::new(&jaw)
        .args(["--kind", "rust"])
        .arg(&output)
        .arg(&schema)
        .status()
        .unwrap_or_else(|error| panic!("failed to run {}: {error}", jaw.display()));

    assert!(status.success(), "JAW generation failed with {status}");
}

fn find_jaw(repo_root: &Path) -> PathBuf {
    if let Some(path) = env::var_os("JAW_BIN") {
        return PathBuf::from(path);
    }

    let adjacent = repo_root.join("../jaw/target/debug/jaw");
    if adjacent.is_file() {
        return adjacent;
    }

    let development = repo_root.join("../../jaw/target/release/jaw");
    if development.is_file() {
        return development;
    }

    PathBuf::from("jaw")
}
