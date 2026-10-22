# Projects and scenes

A **project** is a directory. Its `project.xml` is the manifest: what the project
is called, how its window opens, where its assets live and which **scenes** it is
made of. An `assets.xml` beside it is the asset **registry**: the table that gives
every texture, sound, font and prefab a stable id. A **scene** is a file of its
own: the entities and components that were open in the editor when it was saved.
A **prefab** is one entity saved on its own, so it can be placed into any scene.

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
  Scene version 6 is the second: a texture and a sound reference changed meaning,
  from a name to an asset id. It keeps the same shape, so the migration is the
  resolution order on load -- id, then name, then path -- rather than a rewrite.

Two rules keep the additive case honest:

1. **Component groups are appended, never reordered or removed.** The reader walks
   them in the order the writer emitted them, so a reorder would silently misread
   every older file instead of failing.
2. **Versions are never renumbered after a release.** Before a version ships, its
   number is free to change; once it is out, the number is what makes the file
   readable.

| Milestone | Manifest | Scene | Prefab | Assets |
|---|---|---|---|---|
| today | 3 | 6 | 1 | 1 |
| #54 | 1 | 1 | -- | -- |
| #47 | 1 | **2** | -- | -- |
| #48 | 1 | **3** | -- | -- |
| #49 | 1 | **4** | -- | -- |
| #52 | 1 | **5** | -- | -- |
| #50 | 1 | 5 | **1** | -- |
| #51 | 1 | **6** | 1 | **1** |
| #46b | **2** | 6 | 1 | 1 |
| #46c | **3** | 6 | 1 | 1 |

Every row above is shipped behaviour: the migration policy itself, the first
breaking change made under it, the four additive changes made after it, prefabs,
the asset registry, and the manifest's audio and input sections.

## The manifest

`SampleProject/project.xml`, exactly as the engine writes it:

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<project>
		<formatVersion>3</formatVersion>
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
		<audio>
			<master>128</master>
			<music>128</music>
			<sfx>128</sfx>
		</audio>
		<input size="dynamic">
			<value0>
				<action>editor.gizmo.rotate</action>
				<key>E</key>
			</value0>
			<value1>
				<action>editor.gizmo.scale</action>
				<key>R</key>
			</value1>
			<value2>
				<action>editor.gizmo.translate</action>
				<key>W</key>
			</value2>
			<value3>
				<action>editor.reload</action>
				<key>F2</key>
			</value3>
			<value4>
				<action>editor.save</action>
				<key>Ctrl+S</key>
			</value4>
			<value5>
				<action>editor.scene.next</action>
				<key>Ctrl+Tab</key>
			</value5>
			<value6>
				<action>editor.scene.previous</action>
				<key>Ctrl+Shift+Tab</key>
			</value6>
			<value7>
				<action>game.down</action>
				<key>Down</key>
			</value7>
			<value8>
				<action>game.left</action>
				<key>Left</key>
			</value8>
			<value9>
				<action>game.right</action>
				<key>Right</key>
			</value9>
			<value10>
				<action>game.up</action>
				<key>Up</key>
			</value10>
		</input>
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
| `audio/master`, `audio/music`, `audio/sfx` | The project's mix, each 0-128 like `Sound::SetVolume`. Music and effects sit under the master. Version 2 appended the group; a version 1 manifest opens at 128, the volume it played at. |
| `input` | The project's key bindings: one `action` and `key` per entry. Version 3 appended it. An action the list does not mention keeps the key compiled into `InputMap::Defaults()`; an unknown action is ignored with a warning. |

Every path in the manifest is **relative to the manifest's own directory** and is
stored with forward slashes, so a project directory can be copied or moved and
still opens. Backslashes are accepted on the way in and normalised on the way out.

The audio levels are clamped to 0-128 when the manifest is read. A hand-edited
200 is a gain above one, not something to pass to the mixer; a negative value is
silence.

`<scene>` is a cereal container: `size="dynamic"` means the count is not stored,
and the reader counts the `valueN` children. Hand-editing a manifest means keeping
that shape. The editor is what normally writes the file, so this only matters for
projects created by hand or by a script.

### Input bindings

`input` maps an action to the key that performs it. The actions are the ones
this build uses, so a typo in the manifest is a warning rather than a binding
that quietly does nothing:

| Action | Default | Used by |
|---|---|---|
| `editor.save` | `Ctrl+S` | `File > Save` |
| `editor.reload` | `F2` | `File > Reload Scene` |
| `editor.scene.next` | `Ctrl+Tab` | `Scenes > Next Scene` |
| `editor.scene.previous` | `Ctrl+Shift+Tab` | `Scenes > Previous Scene` |
| `editor.gizmo.translate` | `W` | viewport: move gizmo |
| `editor.gizmo.rotate` | `E` | viewport: rotate gizmo |
| `editor.gizmo.scale` | `R` | viewport: scale gizmo |
| `game.up`, `game.down`, `game.left`, `game.right` | `Up`, `Down`, `Left`, `Right` | the game's camera |

A key is written the way a person would: `"Ctrl+S"`, `"Shift+F2"`,
`"Ctrl+Shift+Alt+A"`, `"Space"`, `"Up"`. Modifier and key names are read
case-insensitively, and `Enter`/`Return`, `Esc`/`Escape`, and `Del`/`Delete` are
the same name. The modifiers are part of the match, not optional: `W` does not
fire while `Ctrl` is held, so binding an action to `W` cannot collide with a
`Ctrl+W` somewhere else.

An action missing from the list keeps its compiled default, so a manifest can
override one key without restating the rest. A key that cannot be read keeps the
default too, and says so. A new project's manifest is written with every default
in it, which is where to look up what the build answers to.

The editor's `Ctrl+S` is also a hard fallback, and so is `F1`: a project that
rebinds `editor.save` -- or binds it to a key that cannot be pressed -- must not
lock the editor out of saving.

### Saving a project

`File > Save` (Ctrl+S or F1 in the editor) writes the open **scene first** and the
manifest **second**, so a manifest never names a scene that is not on disk yet.
Both files are written to a scratch path (`*.tmp`) and renamed into place, so a
failed save leaves the previous file intact.

A project manifest is only written when a project is attached. A run started
without one can open a scene but cannot save a project.

`Edit > Project Settings` opens a panel that edits the three audio levels. The
sliders apply to the mix as they move, so a level is set by ear; the panel's
**Save** writes just the manifest, while **Cancel** puts the levels back and
leaves the file alone.

## The asset registry

`SampleProject/assets.xml`, the table that gives every asset a stable id:

```xml
<?xml version="1.0" encoding="utf-8"?>
<cereal>
	<assets>
		<formatVersion>1</formatVersion>
		<asset size="dynamic">
			<value0>
				<id>09378fbe1b6be4bc</id>
				<name>tex3.png</name>
				<type>texture</type>
				<path>assets/tex3.png</path>
			</value0>
			<value1>
				<id>c47f914a7bba8c37</id>
				<name>effect.wav</name>
				<type>sound</type>
				<path>assets/effect.wav</path>
			</value1>
		</asset>
	</assets>
</cereal>
```

| Field | Meaning |
|---|---|
| `id` | 16 lowercase hex characters, generated when the asset is imported. A scene or prefab stores this, never the path. |
| `name` | The display name the editor shows, and what an older scene may still call the asset. It is the file's name, extension included. |
| `type` | `texture`, `sound`, `font` or `prefab`, inferred from the extension when the asset is imported. |
| `path` | Where the file is, relative to the project directory and stored with forward slashes -- the same rule every other project path follows. `Core::ResolveProjectPath` turns it into a path the OS can open. |

The table exists to make a **rename** an entry update rather than a scene edit.
The id is generated from a random engine and is never derived from the path, so
moving `assets/tex3.png` to `assets/sprites/crate.png` changes `path` and leaves
`id` alone; every scene that stored the id keeps resolving. Two files with the
same name in different folders are two entries with distinct ids.

A reference in a scene or prefab resolves in this order:

1. a registered **id**;
2. a registered **name**;
3. a registered **path**, including a bare file name or a path suffix, so
   `tex3.png` finds `assets/tex3.png`;
4. otherwise the text is a legacy path relative to `assetRoot`, which is what
   every version 5 file stores.

Step 4 is the migration. A scene written before the registry keeps loading
unchanged, and saving it later writes ids for the references the registry knows
and leaves the rest as names. Nothing has to be rewritten in place, and no
runtime version check is involved: the field is a string in both versions.

`Assets > Import Asset...` and the content browser's `Register as Asset` add a
file inside the project and give it a fresh id; `Rename Asset...` renames the
file and updates the entry; `Unregister` drops the entry and keeps the file, so
nothing a user authored is deleted by a table edit. An entry whose file is
missing is not an error either: the reference resolves to a texture at id 0 (a
white sprite) or a silent sound, with the file named in one warning.

The table is loaded when a project opens -- `Core::OpenProject` and the startup
path both do it before any scene -- and `File > New Project...` writes an empty
one. A project without `assets.xml` still opens: the table is simply empty and
every reference takes the legacy path route. Format 1 is the first registry
version. A file newer than the build is refused by `AssetRegistry::Load`; opening
a project logs the reason and continues with an empty table rather than refusing
to open the project.

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

A sprite stores a **reference** to its texture, not the texture itself. At
format 6 that reference is the asset registry's id:
`<Texture>09378fbe1b6be4bc</Texture>`. The id is resolved through
`TextureManager`, which asks the registry which file it names and keeps one
texture per asset, so twelve sprites referencing one image share one upload and
one copy in memory. The id is what makes a rename safe: moving `tex3.png` to
`crate.png` updates one registry entry, and every scene that stored the id keeps
resolving without being edited.

A reference that is not a known id is looked up as an asset **name**, and failing
that it is treated as a **path relative to `assetRoot`** -- exactly what it meant
before the registry existed. That fallback is the version 5 to 6 migration, and
it is why version 6 did not break older files: the field is a string either way,
so the version moved for the meaning and not for the shape. A reference that
resolves to nothing does not fail the load: the texture keeps the reference as
its name, ends at id 0 and the sprite draws white, and the engine logs one
warning per missing reference. An empty `<Texture></Texture>` is a sprite with no
texture at all.

Version 1 stored the texture itself, with cereal's polymorphic wrapper. Version 2
changed the field's shape, so a version 1 scene is **refused** as too old rather
than half-read -- there is no migration. `SceneIO::kFirstFormatVersion` moved to 2
with it, and `UnitTests/fixtures/scene_v1.xml` is kept as the file the refusal is
tested against.

A sound is stored the same way: `<Sound>c47f914a7bba8c37</Sound>` is an asset id
at format 6, resolved through `SoundManager`, which keeps one `Sound` per asset
-- one decoded copy, however many entities play it -- and is matched by name or
by path in an older file, exactly like a texture. The playback settings belong
to the entity, not to the shared sound: `Volume`, `Mute`, `Loop` and `Autoplay`
sit beside the reference, so two entities using one file can have different
volumes, and one can loop while the other plays once. `AudioSystem` applies
them, and starts a sound that asks to autoplay exactly once -- a one-shot is not
restarted on the next frame. A file that cannot be loaded does not fail the load
either: the entity keeps the reference and stays silent.

Version 3 appended the audio group, so a version 2 scene still opens -- the group
is read only when the file says version 3 or newer. Version 4 appended the rigid
bodies for the same reason. Version 5 added the camera node, which is root data
rather than a component group: a version 4 scene has none, so it opens with the
defaults, which are the framing it was drawn with. Version 6 changed what a
texture and a sound reference *mean* -- an asset id rather than a name -- without
changing their shape; the fallback above is what keeps version 5 files opening.
The example above is a version 2 file, which is why it has neither the camera nor
the audio and rigid-body groups, and names its texture; a current file has all
three and stores ids. `UnitTests/fixtures/scene_v2.xml` through `scene_v5.xml`
are the files that pin the additive versions, and `scene_v6.xml` is the current
format, paired with `assets_v1.xml`.

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
  texture inside it is still shared through `TextureManager`, by asset id or by
  name;
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

A prefab's texture and sound references mean what they mean in a scene -- an
asset id, or a name/path that falls back through the registry -- so the prefab
format did not move when scene version 6 changed the meaning. A prefab written by
this build stores ids; an older prefab that names its files still loads, through
the same fallback, and is rewritten with ids the next time it is saved.

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
| Assets > Import Asset... | -- | Asks for a file inside the project and adds it to the asset registry with a new id. Disabled while no project is open. |

`Scenes` is the manifest's scene list as a menu, in the order the manifest stores
it, with a tick next to the open scene:

| Command | Shortcut | Does |
|---|---|---|
| Scenes > \<name\> | -- | Opens that scene. Asking for the scene that is already open does nothing -- `Reload Scene` is how a scene is re-read. |
| Scenes > Next Scene / Previous Scene | Ctrl+Tab, Ctrl+Shift+Tab | Opens the next/previous scene in manifest order, wrapping at the ends. Disabled with fewer than two scenes. |
| Scenes > Save Scene | Ctrl+S | The same command as File > Save: the open scene, then the manifest. |
| Scenes > Save Scene As... | -- | The same command as File > Save Scene As.... |
| Scenes > Reload Scene | F2 | The same command as File > Reload Scene. |
| Scenes > New Scene... | -- | Asks for a path inside the project, writes an empty scene there, adds it to the manifest and opens it. A path the manifest already lists is refused rather than overwritten. |

Switching scenes, creating one and `Next`/`Previous` all go through the same
check: when the open scene has unsaved edits, a modal asks whether to discard
them, and Cancel leaves the scene open and dirty. The menu is disabled while no
project is open, because the scene list is manifest data.

The content browser is rooted at the project directory -- the editor's working
directory while no project is open -- and opens in `scenes/` when the project
has one. Double-clicking a scene's row opens it through the same command as
`Scenes > <name>`, including the unsaved-edits question; double-clicking a
folder enters it, and an image still previews in the details panel.

Right-clicking a scene's row offers `Open Scene`, `Rename...`, `Delete` and
`Show in Explorer`; `New Scene...` is on every row and on the empty space, so a
folder with no files in it is still a place to create one. Renaming keeps the
scene in its folder, updates the manifest entry (and the startup scene if that
is the one renamed), saves the open scene first if it is the one being renamed,
and then forgets that scene -- the path it held no longer exists. Deleting
removes the file and its manifest entry; the startup scene and the open scene
are refused, before any confirmation is shown.

The content browser's details panel is the file-level half of the same commands:
it shows the registry name, type and id of the selected file, offers
`Register as Asset` for a file with no entry, and `Rename Asset...` /
`Unregister` for one that has. Renaming moves the file and keeps the id;
unregistering drops the entry and leaves the file. `Unregister` warns when the
open scene still references the id, because the scene will store an id that no
longer resolves.

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
  assets.xml        the asset registry: tex3.png and effect.wav, with their ids
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
the way it always did; editing the background in the editor's Editor Settings
panel changes the open scene's value, and saving the scene stores it. The unit
tests open both scenes: the sample is the one project that has to keep loading
when the format moves.

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
- There are no input bindings for the mouse or a gamepad: `input` binds keys,
  and the gamepad paths in `InputManager` are still stubs.
- The asset registry is a table, not a watcher: a file moved or renamed outside
  the editor leaves its entry pointing at the old path, and the next load falls
  back to a white texture or a silent sound. There is no auto-import, no folder
  entries and no reference counting; unregistering an asset keeps its file, and
  re-importing it gives it a new id, so a scene that stored the old one will not
  pick the file up again -- its reference has to be re-pointed in the editor.
- A placed prefab is a copy, not a linked instance: editing the prefab file does
  not update entities already placed, and editing a placed entity does not
  update the file.
- The game applies the scene camera when it starts; the game has no runtime scene
  switch yet, and the editor viewport deliberately keeps its own editor camera, so
  it does not preview the scene's camera.
- The editor's session state -- open panels, the layout version, the five most
  recent projects, the last scene, the grid and the gizmo -- is not part of the
  project: it lives in `editor.cfg` in the working directory, and the dock
  layout in `imgui.ini`.
  `File > Open Recent` lists the projects, most recent first; the editor drops
  the ones whose folder has gone when it reads the file, so a moved project is a
  one-time fallback rather than a menu item that fails. Selection and undo
  history are not persisted, because entity ids do not survive a load and there
  is no undo system yet.
- Manifests are on version 3: version 1 is read with the default audio levels and
  the compiled bindings, version 2 with the compiled bindings. `Project::Migrate`
  clamps the levels to 0-128. There is still no manifest change that rewrites
  existing data; the hook is in place for the first one.
- The renderer's default shader is chosen when the window is created, so opening a
  different project at runtime reloads scenes and window settings but keeps the
  shader it started with.
- The content browser does not create or open scenes or prefabs yet, and there is
  no `Scenes` menu built from the manifest's list.
