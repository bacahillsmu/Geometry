#include "Game/Gameplay/UI/UIWidget.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/CPUMesh.hpp"



// -----------------------------------------------------------------------
// UIWidget
// -----------------------------------------------------------------------
UIWidget::UIWidget()
{

}

// -----------------------------------------------------------------------
UIWidget::~UIWidget()
{

}

// -----------------------------------------------------------------------
void UIWidget::SetWorldBounds( const AABB2& worldBounds )
{
	m_worldBounds = worldBounds;
}

// -----------------------------------------------------------------------
void UIWidget::SetVirtualPosition( const Vec4& virtualPosition )
{
	m_virtualPosition = virtualPosition;
}

// -----------------------------------------------------------------------
void UIWidget::SetVirtualSize( const Vec4& virtualSize )
{
	m_virtualSize = virtualSize;
}

// -----------------------------------------------------------------------
void UIWidget::SetPivot( const Vec2& pivot )
{
	m_pivot = pivot;
}

// -----------------------------------------------------------------------
void UIWidget::UpdateBoundsAndPosition()
{
	UpdateBounds();
	UpdatePosition();
}

// -----------------------------------------------------------------------
void UIWidget::UpdateBounds()
{
	// Using the World Bounds and Virtual Size, change Bounds;

	// World Bounds: 1920x1080
	// World Bounds: (0, 0), (1920, 1080)
	// Virtual Size: (1.0, 1.0)

	Vec2 newCenter = Vec2(m_worldBounds.maxs.x * m_virtualPosition.x + m_virtualPosition.z, m_worldBounds.maxs.y * m_virtualPosition.y + m_virtualPosition.w);
	
	float halfExtentX = (m_worldBounds.maxs.x - m_worldBounds.mins.x) * 0.5f;
	float halfExtentY = (m_worldBounds.maxs.y - m_worldBounds.mins.y) * 0.5f;

	// Calculate new Half Extents based on Virtual Size;
	float newHalfExtentX = halfExtentX * m_virtualSize.x + m_virtualSize.z;
	float newHalfExtentY = halfExtentY * m_virtualSize.y + m_virtualSize.w;

	Vec2 mins = Vec2(newCenter.x - newHalfExtentX, newCenter.y - newHalfExtentY);
	Vec2 maxs = Vec2(newCenter.x + newHalfExtentX, newCenter.y + newHalfExtentY);

	AABB2 newBounds = AABB2::MakeFromMinsMaxs(mins, maxs);

	m_position = newCenter;
	m_bounds = newBounds;
}

// -----------------------------------------------------------------------
void UIWidget::UpdatePosition()
{

}

void UIWidget::RenderSelf()
{

}

// -----------------------------------------------------------------------
void UIWidget::ProcessInput( PlayerController& input )
{
	UNUSED(input);
}

// -----------------------------------------------------------------------
void UIWidget::Render()
{
	if(m_show)
	{
		RenderSelf();
		for(auto& child: m_children)
		{
			child->Render();
		}
	}
	
}

// -----------------------------------------------------------------------
void UIWidget::AddChild( UIWidget* childWidget )
{
	m_children.push_back(childWidget);
}

// -----------------------------------------------------------------------
void UIWidget::RemoveChild( UIWidget* childWidget )
{
	UNUSED(childWidget);
}

// -----------------------------------------------------------------------
void UIWidget::SetRadioGroup( UIRadioGroup* radioGroup )
{
	UNUSED(radioGroup);
}

// -----------------------------------------------------------------------
void UIWidget::SetShow( bool show )
{
	m_show = show;
}

// -----------------------------------------------------------------------
void UIWidget::SetSibling( UIWidget* sibling )
{
	m_sibling = sibling;
}

// -----------------------------------------------------------------------
void UIWidget::UpdateChildrenBounds()
{

}

// -----------------------------------------------------------------------
void UIWidget::ProcessChildrenInput( PlayerController& input )
{
	UNUSED(input);
}

// -----------------------------------------------------------------------
void UIWidget::RenderChildren()
{

}

// -----------------------------------------------------------------------
void UIWidget::DestroyChildren()
{

}


// -----------------------------------------------------------------------
// UILabel
// -----------------------------------------------------------------------
UILabel::UILabel()
{

}


// -----------------------------------------------------------------------
UILabel::UILabel( UIWidget* parentWidget )
{
	if(!parentWidget)
	{
		ERROR_AND_DIE("Tried to create a UILabel with no Parent set.");
	}

	m_parent = parentWidget;
	m_uiFlags = UIWIDGETFLAGS_LABEL;
	m_worldBounds = parentWidget->GetBounds();
}

// -----------------------------------------------------------------------
UILabel::~UILabel()
{
	delete m_textureMesh;
	m_textureMesh = nullptr;
}

void UILabel::RenderSelf()
{
	if(m_textureView)
	{
		g_theRenderer->BindModelMatrix( m_textureModel );
		g_theRenderer->BindTextureViewWithSampler(0u, m_textureView);
		g_theRenderer->DrawMesh( m_textureMesh );
	}

	g_theRenderer->BindTextureView( 0u, m_widgetFont->GetTextureView() );

	std::vector<Vertex_PCU> textVerts;
	Vec2 printPosition = GetPosition();
	float height = 200.0f * m_virtualSize.y;
	float width = m_label.size() * height;
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;

	m_widgetFont->AddVertsForText2D(textVerts, printPosition, height, m_label, Rgba::WHITE);
	g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);
	g_theRenderer->BindTextureView(0u, nullptr);
}

// -----------------------------------------------------------------------
void UILabel::SetLabel( std::string label )
{
	m_label = label;
}

void UILabel::SetFont( BitMapFont* widgetFont )
{
	m_widgetFont = widgetFont;
}

void UILabel::SetTextureView( TextureView* textureView )
{
	m_textureView = textureView;

	delete m_textureMesh;
	m_textureMesh = nullptr;

	m_textureMesh = new GPUMesh(g_theRenderer);
	CPUMesh mesh;
	mesh.SetLayout<Vertex_PCU>();
	CPUMeshAddQuad( &mesh, m_bounds);  
	m_textureMesh->CreateFromCPUMesh( &mesh );
	m_textureModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

}

TextureView* UILabel::GetTextureView()
{
	return m_textureView;
}

Matrix4x4& UILabel::GetTextureModel()
{
	return m_textureModel;
}

GPUMesh* UILabel::GetTextureMesh()
{
	return m_textureMesh;
}

// -----------------------------------------------------------------------
// UIButton
// -----------------------------------------------------------------------
UIButton::UIButton()
{

}


// -----------------------------------------------------------------------
UIButton::UIButton( UIWidget* parentWidget )
{
	if(!parentWidget)
	{
		ERROR_AND_DIE("Tried to create a UIButton with no Parent set.");
	}

	m_parent = parentWidget;
	m_uiFlags = UIWIDGETFLAGS_BUTTON;
	m_worldBounds = parentWidget->GetBounds();



}

void UIButton::RenderSelf()
{
	if(m_shouldHighlight || m_isSelected)
	{
		Rgba& colorToUse = m_isSelected ? m_selectColor : m_highlightColor;

		Vec2 bottomLeft = Vec2(m_bounds.mins.x, m_bounds.mins.y);
		Vec2 topLeft = Vec2(m_bounds.mins.x, m_bounds.maxs.y);
		Vec2 topRight = Vec2(m_bounds.maxs.x, m_bounds.maxs.y);
		Vec2 bottomRight = Vec2(m_bounds.maxs.x, m_bounds.mins.y);

		g_theRenderer->BindTextureView( 0u, nullptr );

		std::vector<Vertex_PCU> vertsLine;

		AddVertsForLine2D(vertsLine, bottomLeft, topLeft, 100.0f * m_virtualSize.y, colorToUse);
		g_theRenderer->DrawVertexArray((int)vertsLine.size(), &vertsLine[0]);

		AddVertsForLine2D(vertsLine, topLeft, topRight, 100.0f * m_virtualSize.y, colorToUse);
		g_theRenderer->DrawVertexArray((int)vertsLine.size(), &vertsLine[0]);

		AddVertsForLine2D(vertsLine, topRight, bottomRight, 100.0f * m_virtualSize.y, colorToUse);
		g_theRenderer->DrawVertexArray((int)vertsLine.size(), &vertsLine[0]);

		AddVertsForLine2D(vertsLine, bottomRight, bottomLeft, 100.0f * m_virtualSize.y, colorToUse);
		g_theRenderer->DrawVertexArray((int)vertsLine.size(), &vertsLine[0]);
	}
}

void UIButton::ToggleHighlight()
{
	m_shouldHighlight = !m_shouldHighlight;
}

bool UIButton::GetShouldHighlight()
{
	return m_shouldHighlight;
}

void UIButton::SelectButtonFromRadioGroup()
{
	for(auto& radioButton: m_radioGroup->m_radioButtons)
	{
		radioButton->m_isSelected = false;
	}

	for(auto& radioButton: m_radioGroup->m_radioButtons)
	{
		if(this == radioButton)
		{
			radioButton->m_isSelected = true;
		}
	}
}

void UIButton::SetAction( std::string action )
{
	m_action = action;
}

std::string UIButton::GetAction()
{
	return m_action;
}

void UIButton::SetHighlightColor( Rgba color )
{
	m_highlightColor = color;
}

void UIButton::SetRadioGroup( UIRadioGroup* radioGroup )
{
	m_radioGroup = radioGroup;
}

// -----------------------------------------------------------------------
// UIRadioGroup
// -----------------------------------------------------------------------
UIRadioGroup::UIRadioGroup()
{

}

// -----------------------------------------------------------------------
void UIRadioGroup::AddButtonToRadioGroup( UIButton* button )
{
	m_radioButtons.push_back(button);


}
