"""Read-only fresh-process check of the saved children's cover fixtures.

Run after place_fixtures.py with Tools/run_editor_python.ps1. This reloads the
map and never saves it. Its output is about serialized actors, not the in-memory
actors that the placement script just spawned.
"""

import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import place_fixtures as fixtures


def main():
    unreal.EditorLoadingAndSavingUtils.load_map(fixtures.MAP)
    world = fixtures.editor_world()
    fixtures.prove_the_instrument(world)

    actors = fixtures.editor_actors().get_all_level_actors()
    by_label = {}
    for actor in actors:
        if actor.actor_has_tag(fixtures.TAG):
            by_label.setdefault(actor.get_actor_label(), []).append(actor)

    expected = set()
    for spec in fixtures.PLACEMENTS:
        expected.add(spec["label"])
        if spec["mesh"] in fixtures.PROXY_COVER:
            expected.add(spec["label"] + "_COLLISION")

    if set(by_label) != expected:
        raise RuntimeError(
            f"Saved fixture labels differ: expected {sorted(expected)}, "
            f"got {sorted(by_label)}")
    for label, matches in by_label.items():
        if len(matches) != 1:
            raise RuntimeError(f"Saved {label} occurs {len(matches)} times")

    problems = []
    for spec in fixtures.PLACEMENTS:
        visual = by_label[spec["label"]][0]
        location = visual.get_actor_location()
        if (abs(location.x - spec["xy"][0]) > 0.1 or
                abs(location.y - spec["xy"][1]) > 0.1 or
                abs(location.z - fixtures.FLOOR_Z) > 0.1):
            problems.append(f"{spec['label']}: wrong saved position {location}")

        if spec["mesh"] in fixtures.PROXY_COVER:
            proxy = by_label[spec["label"] + "_COLLISION"][0]
            proxy_location = proxy.get_actor_location()
            if (abs(proxy_location.x - spec["xy"][0]) > 0.1 or
                    abs(proxy_location.y - spec["xy"][1]) > 0.1 or
                    abs(proxy_location.z -
                        (fixtures.FLOOR_Z + spec["height"] / 2.0)) > 0.1):
                problems.append(
                    f"{spec['label']}: wrong proxy position {proxy_location}")
            scale = proxy.get_actor_scale3d()
            expected_scale = (spec["size"][0] / 100.0,
                              spec["size"][1] / 100.0,
                              spec["height"] / 100.0)
            if any(abs(a - b) > 0.001 for a, b in zip(
                    (scale.x, scale.y, scale.z), expected_scale)):
                problems.append(f"{spec['label']}: wrong proxy scale {scale}")
            if str(proxy.static_mesh_component.get_collision_profile_name()) != "BlockAll":
                problems.append(f"{spec['label']}: proxy is not BlockAll")
            if not proxy.get_editor_property("hidden"):
                problems.append(f"{spec['label']}: collision proxy visible in game")
            if str(visual.static_mesh_component.get_collision_profile_name()) != "NoCollision":
                problems.append(f"{spec['label']}: unreliable visual body still collides")

        problems.extend(fixtures.probe(world, spec))

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        raise RuntimeError(f"Saved cover failed {len(problems)} checks")
    unreal.log(
        "SAVED COVER VERIFIED: 3 fixtures, 2 collision proxies, exact actor "
        "counts and positions, five agreeing Visibility traces per required "
        "height and open control, after an independent map reload")


if __name__ == "__main__":
    main()
