# Projects and scenes

A **project** is a directory. Its `project.xml` is the manifest: what the project
is called, how its window opens, where its assets live and which **scenes** it is
made of. A **scene** is a file of its own: the entities and components that were
open in the editor when it was saved.

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
build is refused, because the build cannot know what its fields mean; a file older
than the oldest supported version is not that kind of file at all.

Two kinds of change exist, and they are not equal:

* **Additive** -- a new trailing field, or a new trailing component group. Bump
  the version, read the new field only when the file says it is at least that
  version, and leave the default in place otherwise. Older files keep opening.
  This is the default, and the only free kind of change.
* **Breaking** -- a field changes shape, or what a reference points at changes.
  This needs a migration that rewrites the data on load (`Project::Migrate`, and
  the equivalent in `SceneIO`), or a refusal. Best-effort parsing is not an
  option: a file read as the wrong shape is worse than one that will not open.

Two rules keep the additive case honest:

1. **Component groups are appended, never reordered or removed.** The reader walks
   them in the order the writer emitted them, so a reorder would silently misread
   every older file instead of failing.
2. **Versions are never renumbered after a release.** Before a version ships, its
   number is free to change; once it is out, the number is what makes the file
   readable.

| Milestone | Manifest | Scene | Prefab | Assets |
|---|---|---|---|---|
| today | 1 | 1 | -- | -- |
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
increments land in, one issue at a time. The first two milestone rows are the
migration policy itself and the first use of it.

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
		</scene>
	</project>
</cereal>
```

| Element | Meaning |
|---|---|
| `formatVersion` | Version of the manifest format. Checked before anything else is read: older versions are migrated forward, newer ones are refused. |
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

A scene file is the entity list: an `entt` snapshot of the registry, wrapped in a
`formatVersion` header and an `<entities>` block.

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<formatVersion>1</formatVersion>
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
							<Texture>
								<ptr_wrapper>
									<id>0</id>
								</ptr_wrapper>
							</Texture>
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
file readable at all. **Scene files are machine-written**: the editor and the
engine's own save are the intended ways to produce them.

A scene's name is its file stem: `scenes/start.xml` is "start" everywhere in the UI,
and two scenes cannot have the same name inside one project because they cannot
have the same path.

### What a scene stores

| Stored | Not stored (yet) |
|---|---|
| `BasicComponent` (name, static flag, tag, layer) | `AudioComponent` |
| `TransformComponent` (position, rotation, scale) | `RigidBodyComponent` |
| `SpriteComponent` (size, colour, texture) | |

Audio and rigid bodies have cereal hooks of their own, but they are not
round-trip safe yet, so they are deliberately left out. An entity that carries one
still saves; the engine logs one warning per save counting the entities whose
extra components were dropped, instead of losing them silently.

The three groups above are written in that order and read in that order. Anything
added later is appended as a fourth group, read only from the version that
introduced it; see [Versions](#versions).

Textures are stored by value inside the sprite (a texture id and its dimensions,
plus cereal's polymorphic wrapper), not by name. A texture that is not loaded when
the scene is loaded cannot be resolved, which is why the sample scene uses
coloured sprites and no texture references.

## The editor's project commands

| Command | Shortcut | Does |
|---|---|---|
| File > New Project... | -- | Asks for a folder, writes `project.xml`, an empty `scenes/start.xml` and an empty `assets/`, then opens it. |
| File > Open Project... | -- | Asks for a `project.xml`, retitles and resizes the window, opens its startup scene. |
| File > Save | Ctrl+S, F1 | Saves the open scene, then the manifest. |
| File > Save Scene As... | -- | Writes the open entities to a new scene inside the project, adds it to the manifest and makes it the open scene. |
| File > Reload Scene | F2 | Re-reads the open scene from disk, discarding unsaved edits. |

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
  scenes/
    start.xml         12 coloured sprites in a 4x3 grid
```

The unit tests open it: the sample is the one project that has to keep loading when
the format moves.

## Limits and follow-ups

- Textures are stored by value; the next step is storing them by name and loading
  them from `assetRoot` through `TextureManager`.
- `AudioComponent` and `RigidBodyComponent` are not persisted (see above).
- There are no prefabs, no asset ids and no input bindings in the manifest.
- Camera and background are per-run, not per-scene.
- The editor's session state (open panels, last scene, selection) is not part of
  the project; it still lives in `imgui.ini` and `default.cfg`.
- There is no recent-projects list. Version 1 is the only published version of
  each file, so there is no migration to exercise yet -- the range check and the
  `Migrate` hook are in place for the first one that needs it.
- The renderer's default shader is chosen when the window is created, so opening a
  different project at runtime reloads scenes and window settings but keeps the
  shader it started with.
- The content browser does not create or open scenes yet, and there is no
  `Scenes` menu built from the manifest's list.
