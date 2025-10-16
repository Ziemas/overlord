import argparse


def apply(config, args):
    config["arch"] = "mipsel"
    config["baseimg"] = f"in/OVERLORD.IRX"
    config["myimg"] = f"build/OVERLORD.IRX"
    config["mapfile"] = f"build/OVERLORD.IRX.map"
    config["source_directories"] = [
        "src",
        "asm",
        "include",
        "assets",
    ]
    config["objdump_flags"] = ["-Mreg-names=n32"]
    config["expected_dir"] = f"expected/"
