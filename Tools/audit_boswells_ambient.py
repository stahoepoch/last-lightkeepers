"""Read only: why the sales floor is black.

The 2026-11-22 screenshots showed a store that is not empty - 1,638 fixtures
stand in it - but whose departments are pure black either side of a lit
racetrack aisle. `ROADMAP.md` slice 2 requires that "the chapter remains
navigable with the lantern off", and it currently is not.

This asks what, if anything, lights the room as a room: a sky light, a
directional, fog, and what the post-process volume is doing to exposure. The
first census missed sky lights entirely because `ASkyLight` derives from
`AInfo` rather than `ALight`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("AMBIENT map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()


def count(cls):
    return [a for a in actors if isinstance(a, cls)]


for name, cls in (
    ("SkyLight", unreal.SkyLight),
    ("DirectionalLight", unreal.DirectionalLight),
    ("ExponentialHeightFog", unreal.ExponentialHeightFog),
    ("SkyAtmosphere", unreal.SkyAtmosphere),
    ("PostProcessVolume", unreal.PostProcessVolume),
    ("SpotLight", unreal.SpotLight),
    ("PointLight", unreal.PointLight),
    ("RectLight", unreal.RectLight),
):
    found = count(cls)
    print("AMBIENT %s=%d %s" % (
        name, len(found), [a.get_actor_label() for a in found][:6]))

# What the post-process volume is doing to exposure, which can make a
# perfectly lit room look like this one.
for volume in count(unreal.PostProcessVolume):
    settings = volume.get_editor_property("settings")
    print("AMBIENT PPV %s unbound=%s priority=%s" % (
        volume.get_actor_label(),
        volume.get_editor_property("unbound"),
        volume.get_editor_property("priority")))
    for prop in ("auto_exposure_method", "auto_exposure_min_brightness",
                 "auto_exposure_max_brightness", "auto_exposure_bias",
                 "override_auto_exposure_min_brightness",
                 "override_auto_exposure_max_brightness",
                 "override_auto_exposure_bias"):
        try:
            print("AMBIENT PPV %s=%s" % (prop, settings.get_editor_property(prop)))
        except Exception as error:  # noqa: BLE001 - reporting, not controlling
            print("AMBIENT PPV %s=<unreadable %s>" % (prop, error))

# Total authored light, so "add more lights" can be argued against numbers.
total = 0.0
for light in actors:
    comp = None
    if isinstance(light, unreal.SpotLight):
        comp = light.spot_light_component
    elif isinstance(light, unreal.PointLight):
        comp = light.point_light_component
    if comp is None:
        continue
    try:
        total += float(comp.get_editor_property("intensity"))
    except Exception:  # noqa: BLE001
        pass
print("AMBIENT total_authored_intensity=%.0f" % total)

# And how much of the sales floor is further than a few metres from any of
# them, which is the figure that decides whether the departments can be read.
lights = [a for a in actors
          if isinstance(a, (unreal.SpotLight, unreal.PointLight))]
floor = next((a for a in actors
              if a.get_actor_label() == "ANCHOR_FULL_FLOOR_LOWER"), None)
if floor and lights:
    center, extent = floor.get_actor_bounds(False)
    step = 300.0
    lit = dark = 0
    x = center.x - extent.x
    while x <= center.x + extent.x:
        y = center.y - extent.y
        while y <= center.y + extent.y:
            near = min(
                (unreal.Vector(x, y, 150.0) - l.get_actor_location()).length()
                for l in lights)
            if near <= 900.0:
                lit += 1
            else:
                dark += 1
            y += step
        x += step
    total_pts = lit + dark
    print("AMBIENT floor_samples=%d within_9m_of_a_light=%d (%.0f%%) beyond=%d (%.0f%%)" % (
        total_pts, lit, lit * 100.0 / total_pts, dark, dark * 100.0 / total_pts))
