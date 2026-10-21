# Projects and scenes

A **project** is a directory. Its `project.xml` is the manifest: what the project
is called, how its window opens, where its assets live and which **scenes** it is
made of. A **scene** is a file of its own: the entities and components that were
open in the editor when it was saved. A **prefab** is one entity saved on its own,
so it can be placed into any scene.

Before this split, `project.xml` *was* the scene: a flat list of entities written
into whatever directory the binary happened to be run from. A project that could
grow -- several levels, a window size, an asset folder -- needed a file that
outlives any one of them.

## Running a project

```bat
Game.exe SampleProject\project.xml
Editor.exe SampleProject\project.xml
```

Both binaries take the manifest's path as their first argument and fall back to
`./project.xml`, so the usual way to run the sample is from the folder that holds
it:

```bat
cd bin\x64-debug
Game.exe SampleProject\project.xml
```

Running with no argument and no `project.xml` next to the binary is supported: the
engine logs `No project at project.xml; running without one.` and starts with an
empty scene and the default window. A project file that exists but cannot be read
is an error (`Failed to open the project: ...`), and the same thing happens --
nothing is written over it.

## Versions

Every file a project is made of carries its own `formatVersion`, and the versions
are independent: a manifest, a scene, a prefab and the asset registry move on
their own schedule. A build reads a file when its version is between the oldest
one it still understands and the newest one it writes. A file **newer** than the
build is refused, because the build cannot know what its fields mean. A file
**older** than the oldest version the build reads is refused too, with its own
message: below version 1 it is not that kind of file at all, and between 1 and the
oldest supported version it is one of ours that is too old to read.

Two kinds of change exist, and they are not equal:

* **Additive** -- a new trailing field, or a new trailing component group. Bump
  the version, read the new field only when the file says it is at least that
  version, and leave the default in place otherwise. Older files keep opening.
  This is the default, and the only free kind of change.
* **Breaking** -- a field changes shape, or what a reference points at changes.
  This needs a migration that rewrites the data on load (`Project::Migrate`, and
  the equivalent in `SceneIO`), or a refusal: the oldest supported version moves
  up to the new one and older files are refused as too old. Best-effort parsing
  is not an option: a file read as the wrong shape is worse than one that will
  not open. Scene version 2 is the first breaking change; see [Scene files](#scene-files).

Two rules keep the additive case honest:

1. **Component groups are appended, never reordered or removed.** The reader walks
   them in the order the writer emitted them, so a reorder would silently misread
   every older file instead of failing.
2. **Versions are never renumbered after a release.** Before a version ships, its
   number is free to change; once it is out, the number is what makes the file
   readable.

| Milestone | Manifest | Scene | Prefab | Assets |
|---|---|---|---|---|
| today | 1 | 5 | 1 | -- |
| #54 | 1 | 1 | -- | -- |
| #47 | 1 | **2** | -- | -- |
| #48 | 1 | **3** | -- | -- |
| #49 | 1 | **4** | -- | -- |
| #52 | 1 | **5** | -- | -- |
| #50 | 1 | 5 | **1** | -- |
| #51 | 1 | **6** | 1 | **1** |
| #46b | **2** | 6 | 1 | 1 |
| #46c | **3** | 6 | 1 | 1 |

Only the rows up to today are shipped behaviour; the rest is the order the
increments land in, one issue at a time. The first seven rows are shipped: the
migration policy itself, the first breaking change made under it, the four
additive changes made after it, and prefabs.

## The manifest

`SampleProject/project.xml`, exactly as the engine writes it:

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<project>
		<formatVersion>1</formatVersion>
		<name>Sample</name>
		<engineVersion>0.13</engineVersion>
		<assetRoot>assets</assetRoot>
		<window>
			<title>Sample Project</title>
			<width>1280</width>
			<height>720</height>
		</window>
		<startupScene>scenes/start.xml</startupScene>
		<scene size="dynamic">
			<value0>scenes/start.xml</value0>
			<value1>scenes/physics.xml</value1>
		</scene>
	</project>
</cereal>
```

| Element | Meaning |
|---|---|
| `formatVersion` | Version of the manifest format. Checked before anything else is read: versions older than this build supports are migrated forward or refused, newer ones are refused. |
| `name` | The project's name. The editor shows it in the title bar. |
| `engineVersion` | The engine version that wrote the file. Informational: only `formatVersion` decides whether the file can be read. `Save` stamps this build's version, so it names the last thing that wrote the file. |
| `assetRoot` | Directory, relative to the manifest, where the project's assets live. The renderer looks here for its default shader before falling back to the engine's own copy. |
| `window/title`, `window/width`, `window/height` | The window the project opens with. |
| `startupScene` | The scene opened at startup. It has to be one of `scene`. |
| `scene` | The project's scenes, in order. Each entry is a path relative to the manifest. |

Every path in the manifest is **relative to the manifest's own directory** and is
stored with forward slashes, so a project directory can be copied or moved and
still opens. Backslashes are accepted on the way in and normalised on the way out.

`<scene>` is a cereal container: `size="dynamic"` means the count is not stored,
and the reader counts the `valueN` children. Hand-editing a manifest means keeping
that shape. The editor is what normally writes the file, so this only matters for
projects created by hand or by a script.

### Saving a project

`File > Save` (Ctrl+S or F1 in the editor) writes the open **scene first** and the
manifest **second**, so a manifest never names a scene that is not on disk yet.
Both files are written to a scratch path (`*.tmp`) and renamed into place, so a
failed save leaves the previous file intact.

A project manifest is only written when a project is attached. A run started
without one can open a scene but cannot save a project.

## Scene files

A scene file is an `entt` snapshot of the registry -- an `<entities>` block --
wrapped in a `formatVersion` header and the scene's own `<camera>`.

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<formatVersion>2</formatVersion>
	<entities>
		<value0>12</value0>
		<value1>11</value1>
		...
		<value15>
			<Name>sprite_0</Name>
			<Static>true</Static>
			<Tag>0</Tag>
			<Layer>0</Layer>
		</value15>
		...
		<value40>
			<Position>
				<x>240</x>
				<y>160</y>
				<z>0</z>
			</Position>
			<Rotation>...</Rotation>
			<Scale>...</Scale>
		</value40>
		...
		<value65>
			<Sprite>
				<polymorphic_id>1073741824</polymorphic_id>
				<ptr_wrapper>
					<id>2147483649</id>
					<data>
						<value0>
							<Size>
								<x>80</x>
								<y>80</y>
							</Size>
							<Color>4278255360</Color>
							<Texture>tex3.png</Texture>
						</value0>
						<Position>
							<x>240</x>
							<y>160</y>
							<z>0</z>
						</Position>
					</data>
				</ptr_wrapper>
			</Sprite>
		</value65>
	</entities>
</cereal>
```

The `valueN` element names are entt's: it writes entities and components as
unnamed nodes, and cereal numbers unnamed nodes. The components inside them keep
their own names (`Name`, `Position`, `Size`, `Color`, ...), which is what makes the
file readable at all. `formatVersion`, `camera` and `entities` are the three named
siblings at the root. **Scene files are machine-written**: the editor and the
engine's own save are the intended ways to produce them.

A scene's name is its file stem: `scenes/start.xml` is "start" everywhere in the UI,
and two scenes cannot have the same name inside one project because they cannot
have the same path.

### What a scene stores

A scene stores every component an entity can carry. Each one has a cereal hook
that writes data, never a pointer, so saving and loading is a round trip:

- `BasicComponent` -- name, static flag, tag, layer.
- `TransformComponent` -- position, rotation, scale.
- `SpriteComponent` -- size, colour, texture name.
- `AudioComponent` -- sound name, volume, mute, loop, autoplay.
- `RigidBodyComponent` -- position, size, density, friction, body type.

The five component groups above are written in that order and read in that
order, after the scene's camera. Anything added later is appended as a sixth
group, read only from the version that introduced it; see [Versions](#versions).

A scene also stores its **camera**, which is scene-root data rather than a
component: there is one per scene, and it sits beside `<entities>` so a reader
meets it before the entities it frames. It says where the 2D view looks, how far
in it is zoomed and what colour the frame is cleared to:

```xml
	<camera>
		<Position>
			<x>640</x>
			<y>360</y>
		</Position>
		<Zoom>1</Zoom>
		<Background>
			<x>1</x>
			<y>0</y>
			<z>1</z>
		</Background>
	</camera>
```

`Position` is the world point that lands in the middle of the frame, `Zoom` is
how much of the 1280x720 reference rectangle is shown (1 shows all of it), and
`Background` is the colour the renderer clears to every frame. Those three values
are what the engine framed and cleared to before a scene could say, so a scene
that does not set them looks exactly as it did. The game applies the camera once
when it starts and then leaves the arrow keys alone; the editor viewport keeps
its own editor camera and only uses the background.

A sprite stores its texture by **name**, not by value: `<Texture>tex3.png</Texture>`
is a path relative to `assetRoot`. The name is resolved through `TextureManager`
the first time a scene that uses it is loaded, and the manager keeps one texture
per name, so twelve sprites naming one image share one upload and one copy in
memory. A name that cannot be loaded does not fail the load: the texture keeps its
name, ends at id 0 and the sprite draws white, and the engine logs one warning per
missing name. An empty `<Texture></Texture>` is a sprite with no texture at all.

Version 1 stored the texture itself, with cereal's polymorphic wrapper. Version 2
changed the field's shape, so a version 1 scene is **refused** as too old rather
than half-read -- there is no migration. `SceneIO::kFirstFormatVersion` moved to 2
with it, and `UnitTests/fixtures/scene_v1.xml` is kept as the file the refusal is
tested against.

A sound is stored by **name** in the same way: `<Sound>effect.wav</Sound>` is
resolved through `SoundManager`, which keeps one `Sound` per name -- one decoded
copy, however many entities play it. The playback settings belong to the entity,
not to the shared sound: `Volume`, `Mute`, `Loop` and `Autoplay` sit beside the
name, so two entities using one file can have different volumes, and one can loop
while the other plays once. `AudioSystem` applies them, and starts a sound that
asks to autoplay exactly once -- a one-shot is not restarted on the next frame.
A file that cannot be loaded does not fail the load either: the entity keeps its
name and stays silent.

Version 3 appended the audio group, so a version 2 scene still opens -- the group
is read only when the file says version 3 or newer. Version 4 appended the rigid
bodies for the same reason. Version 5 added the camera node, which is root data
rather than a component group: a version 4 scene has none, so it opens with the
defaults, which are the framing it was drawn with. The example above is a version
2 file, which is why it has neither the camera nor the audio and rigid-body
groups; a current file has all three. `UnitTests/fixtures/scene_v3.xml`,
`scene_v4.xml` and `scene_v5.xml` are the files that pin this, and `scene_v2.xml`
is the one that has to keep opening.

A rigid body is stored as its **inputs**, by value: position, size, density,
friction and the body type by name (`static`, `kinematic`, `dynamic`). A
`b2Body` is a simulation object that belongs to a world, not data, so it is not
in the file at all: neither is "initialised". A loaded body therefore starts in
the same state as one added in the editor, and `PhysicsSystem` creates the
`b2Body` on its first update. A body is per entity, so there is no manager to
resolve it against -- unlike a texture or a sound.

## Prefab files

A prefab is one entity saved on its own, so it can be placed into any scene. Its
file is a `formatVersion` header and one `<entity>`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<formatVersion>1</formatVersion>
	<entity>
		<value0>1</value0>
		...
	</entity>
</cereal>
```

`<entity>` is an entt snapshot of one entity, exactly like a scene's
`<entities>`: the same `valueN` names and the same component groups in the same
order. A prefab therefore stores everything a scene stores for an entity --
name, transform, sprite, audio settings and rigid-body inputs -- and a component
group appended to scenes later is a prefab group the moment it is appended here
too. A prefab's format version moves independently of a scene's.

The entity ids in the file are **not restored**. `PrefabIO::Instantiate` reads
the whole file into a scratch registry first, then copies each entity into the
open scene with an id the scene hands out. This is deliberate: entt's
`restore()` keeps the ids from the file, and a prefab saved from a scene would
otherwise overwrite whatever the scene already has at those ids. It also means
a failed load cannot half-replace the scene -- the header and every component
are read before the first entity is copied.

The copy owns what the entity owns:

- a sprite is copied, so changing one placed copy does not change another; the
  texture inside it is still shared by name through `TextureManager`;
- a rigid body is cloned as its inputs, uninitialised, so `PhysicsSystem` builds
  a fresh `b2Body` for each placement;
- a sound is borrowed from `SoundManager`, which is what a sound is; the
  per-entity volume, mute, loop and autoplay settings are copied with it.

A prefab is **not linked** to what it places. Editing the file does not change
entities already placed, and editing a placed entity does not change the file.
Linked instances would be a separate feature.

Prefabs live in the project's `prefabs/` directory. They are not listed in
`project.xml`: the manifest is version 1 and has no prefab list, so a prefab is
found by path -- through the editor today, and through the content browser when
it learns about prefabs. Saving one still has to happen inside the project
directory, for the same reason a scene does. Format 1 is the first prefab
version; a file newer than the build is refused before the open scene is
touched, like every other format.

## The editor's project commands

| Command | Shortcut | Does |
|---|---|---|
| File > New Project... | -- | Asks for a folder, writes `project.xml`, an empty `scenes/start.xml`, an empty `assets/` and an empty `prefabs/`, then opens it. |
| File > Open Project... | -- | Asks for a `project.xml`, retitles and resizes the window, opens its startup scene. |
| File > Save | Ctrl+S, F1 | Saves the open scene, then the manifest. |
| File > Save Scene As... | -- | Writes the open entities to a new scene inside the project, adds it to the manifest and makes it the open scene. |
| File > Reload Scene | F2 | Re-reads the open scene from disk, discarding unsaved edits. |
| Assets > Create Prefab from Selection... | -- | Asks for a path inside the project and writes the selected entity to it as a prefab. Disabled while nothing is selected. |
| Assets > Instantiate Prefab... | -- | Asks for a prefab and copies it into the open scene at the positions it stores, then marks the scene dirty. |

The file and folder pickers are SDL's native dialogs. They are asynchronous: the
chosen path arrives during the event pump, not when the menu item is clicked. When
a platform has no dialog, or the dialog cannot be shown, the command opens a
typed-path prompt instead.

The title bar reads `* <scene> - <project> - RockSlide Engine`, with the `*` while
the open scene has unsaved edits (entity creation or deletion, component add or
remove, inspector edits, gizmo drags).

## The sample project

`SampleProject/` is checked in and deployed next to the binaries as a directory of
its own, so `Game.exe SampleProject\project.xml` works from `bin\<config>` out of
the box:

```
SampleProject/
  project.xml
  assets/
    Default.vert
    Default.frag
    tex3.png
    effect.wav
  prefabs/
    crate.xml         the physics scene's dynamic crate, on its own
  scenes/
    start.xml         12 sprites in a 4x3 grid, all using tex3.png
    physics.xml       one static floor and one dynamic crate
```

The sprites keep their twelve colours and share one image, so the scene is also
the worked example of the texture rule: one asset, one texture, twelve sprites.
`sprite_0` also carries an `AudioComponent` for `effect.wav`, with autoplay off:
the scene is the worked example of the audio rule -- the file is named once and
the playback settings live on the entity, so turning autoplay on in the inspector
plays it.

`physics.xml` is the second scene, listed in the manifest, and the worked example
of the rigid-body rule: the floor is a static body, the crate a dynamic one. Both
store their inputs; neither stores a `b2Body`, so the crate starts falling as
soon as the scene is open and stepping.

Both sample scenes carry the default camera and background, so the sample looks
the way it always did; editing the background in the editor's Debug panel changes
the open scene's value, and saving the scene stores it. The unit tests open both
scenes: the sample is the one project that has to keep loading when the format
moves.

`prefabs/crate.xml` is the physics crate saved with
`Assets > Create Prefab from Selection`, and the worked example of the prefab
rule: instantiating it places another dynamic crate at the file's own position,
with its own sprite and its own uninitialised body. Nothing links the copy back
to the file, which is why the sample can keep the file and two scenes without
keeping them in step.

## Limits and follow-ups

- Audio is 2D and one mixer track per sound: two entities that play the same file
  at the same time share that track, so the gain applied is the last entity's.
  The settings themselves are per entity and survive a save.
- Physics is one world with fixed gravity and a fixed time step, owned by
  `PhysicsSystem`; there is no way to change either from a scene yet.
- There is no asset registry and there are no input bindings in the manifest.
- A placed prefab is a copy, not a linked instance: editing the prefab file does
  not update entities already placed, and editing a placed entity does not
  update the file.
- The game applies the scene camera when it starts; the game has no runtime scene
  switch yet, and the editor viewport deliberately keeps its own editor camera, so
  it does not preview the scene's camera.
- The editor's session state (open panels, last scene, selection) is not part of
  the project; it still lives in `imgui.ini` and `default.cfg`.
- There is no recent-projects list.
- Manifests are still on version 1, the only version published, so
  `Project::Migrate` has no blocks yet -- the range check and the hook are in place
  for the first manifest change that needs one. Scenes have had their first
  breaking change and refuse version 1 outright; see above.
- The renderer's default shader is chosen when the window is created, so opening a
  different project at runtime reloads scenes and window settings but keeps the
  shader it started with.
- The content browser does not create or open scenes or prefabs yet, and there is
  no `Scenes` menu built from the manifest's list.
