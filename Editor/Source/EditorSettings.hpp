#pragma once

// ---------------------------------------------------------------------------
// Editor settings.
//
// The state behind View > Show Grid, kept in one struct so that M7 can write the
// whole thing to editor.cfg without anything having to be re-plumbed. Only what
// something actually reads lives here: a settings field nobody uses is a field
// that quietly rots.
// ---------------------------------------------------------------------------
struct EditorSettings
{
	struct Grid
	{
		bool visible{ true };

		// World units across one grid square, and the quantum snapping rounds
		// to. Both uses are the same number on purpose: a grid you can see has
		// to be the grid you snap to.
		float base{ 32.0f };
	};

	// Which gizmo the viewport draws. The viewport HUD reports it and the
	// W/E/R keys set it; the gizmos themselves arrive in M6.
	enum class GizmoMode
	{
		Translate,
		Rotate,
		Scale
	};

	struct Gizmo
	{
		GizmoMode mode{ GizmoMode::Translate };
	};

	Grid grid;
	Gizmo gizmo;
};

inline const char* gizmoModeName(EditorSettings::GizmoMode mode)
{
	switch (mode)
	{
		case EditorSettings::GizmoMode::Rotate: return "rotate";
		case EditorSettings::GizmoMode::Scale: return "scale";
		case EditorSettings::GizmoMode::Translate: break;
	}

	return "translate";
}
