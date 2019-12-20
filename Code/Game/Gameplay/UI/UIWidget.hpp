#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>

class BitMapFont;
class PlayerController;
class UIRadioGroup;
class TextureView;

enum UIWidgetFlags
{
	UIWIDGETFLAGS_UNKNOWN = -1,

	UIWIDGETFLAGS_LABEL,
	UIWIDGETFLAGS_BUTTON

};

class UIWidget
{

public:

	UIWidget();
	virtual ~UIWidget();

	void Render();
	
	// Input;
	void ProcessInput( PlayerController& input);
	
	// Children;
	void AddChild( UIWidget* childWidget );
	void RemoveChild( UIWidget* childWidget );

	// Get;
	inline const Vec2& GetPosition() { return m_position; }
	inline AABB2& GetBounds() { return m_bounds; }
	inline const std::vector<UIWidget*>& GetChildren() { return m_children; }
	inline const UIWidgetFlags& GetUIFlags() { return m_uiFlags; }
	inline UIWidget* GetSibling(){ return m_sibling; }

	// Set;
	void SetWorldBounds( const AABB2& worldBounds );
	void SetVirtualPosition( const Vec4& virtualPosition );
	void SetVirtualSize( const Vec4& virtualSize );
	void SetPivot( const Vec2& pivot );
	void SetRadioGroup( UIRadioGroup* radioGroup );
	void SetShow(bool show);
	void SetSibling(UIWidget* sibling);

	// Update;
	void UpdateBoundsAndPosition();
	void UpdateBounds();
	void UpdatePosition();

	// Misc;
	inline void ToggleShow(){ m_show = !m_show; }


public:

	template <typename T>
	T* CreateChild()
	{
		// Create a child of Type T and add it to Children;

	}


protected:

	virtual void RenderSelf();

	// Heirarchy Information;
	UIWidget* m_parent = nullptr;
	UIWidget* m_sibling = nullptr;
	std::vector<UIWidget*> m_children;

	// Independent Variables;
	Vec4 m_virtualPosition = Vec4( 0.5f, 0.5f, 0.0f, 0.0f );
	Vec4 m_virtualSize = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );
	AABB2 m_worldBounds = AABB2( Vec2( -1.0f, -1.0f ), Vec2( -1.0f, -1.0f ) );
	Vec2 m_pivot = Vec2( 0.5f, 0.5f );

	// Derived from above;
	Vec2 m_position = Vec2( -1.0f, -1.0f );
	AABB2 m_bounds = AABB2( Vec2( -1.0f, -1.0f ), Vec2( -1.0f, -1.0f ) );

	// Misc Data;
	bool m_show = true;
	UIWidgetFlags m_uiFlags = UIWIDGETFLAGS_UNKNOWN;
	UIRadioGroup* m_radioGroup = nullptr;


private:

	void UpdateChildrenBounds(); // Will update children using my bounds as their container;
	void ProcessChildrenInput( PlayerController& input ); // Update input - will process backwards;
	void RenderChildren(); // Render Children - will process forwards;
	void DestroyChildren(); // Kill children when we done;

};

class UILabel : public UIWidget
{
	
public:

	UILabel();
	UILabel(UIWidget* parentWidget);
	~UILabel();

	virtual void RenderSelf();

	void SetLabel(std::string label);
	void SetFont(BitMapFont* widgetFont);
	void SetTextureView( TextureView* textureView );
	TextureView* GetTextureView();
	Matrix4x4& GetTextureModel();
	GPUMesh* GetTextureMesh();

public:

	std::string m_label = "not set";
	BitMapFont* m_widgetFont = nullptr;
	TextureView* m_textureView = nullptr;

	// Mesh;
	GPUMesh* m_textureMesh = nullptr;

	// Models;
	Matrix4x4 m_textureModel;

};

class UIButton : public UIWidget
{

public:

	UIButton();
	UIButton(UIWidget* parentWidget);

	virtual void RenderSelf();

	void ToggleHighlight();
	bool GetShouldHighlight();

	void SelectButtonFromRadioGroup();

	void SetAction(std::string action);
	std::string GetAction();

	void SetHighlightColor(Rgba color);

	void SetRadioGroup(UIRadioGroup* radioGroup);

public:

	bool m_isSelected = false;
	bool m_shouldHighlight = false;
	Rgba m_highlightColor = Rgba::YELLOW;
	Rgba m_selectColor = Rgba::WHITE;
	std::string m_action = "none";
	UIRadioGroup* m_radioGroup = nullptr;

};

class UIRadioGroup
{

public:

	UIRadioGroup();

	void AddButtonToRadioGroup(UIButton* button);


	std::vector<UIButton*> m_radioButtons;

};