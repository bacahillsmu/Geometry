#include "Game/Shapes/ConvexPoly.hpp"


// -----------------------------------------------------------------------
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include "Game/Framework/App.hpp"
#include "Game/Gameplay/Game.hpp"



// -----------------------------------------------------------------------
ConvexPoly2D::ConvexPoly2D()
{

}

// -----------------------------------------------------------------------
ConvexPoly2D::~ConvexPoly2D()
{

}

// -----------------------------------------------------------------------
void ConvexPoly2D::Render()
{
	std::vector<Vertex_PCU> convexPoly2DVerts;

	bool inside = IsPointInside(g_theApp->m_theGame->m_worldMousePosition);

	if (inside)
	{
		AddVertsForConvexPoly2D(convexPoly2DVerts, *this, Rgba::GRAY);
	}
	else
	{
		AddVertsForConvexPoly2D(convexPoly2DVerts, *this, Rgba::TEAL);
	}

	if(g_theApp->m_theGame->m_shouldShowSurfaceNormals)
	{
		for (const Line& edge : m_edges)
		{
			Vec2 center = (edge.lineEnd + edge.lineStart) / 2;
			AddVertsForRay2D(convexPoly2DVerts, center, center + (edge.m_surfaceNormal * 2.0f), 0.25f, 10.0f, Rgba::MAGENTA);
		}
	}

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)convexPoly2DVerts.size(), &convexPoly2DVerts[0]);
}

void ConvexPoly2D::SpecialRender()
{
	std::vector<Vertex_PCU> convexPoly2DVerts;
	AddVertsForConvexPoly2D(convexPoly2DVerts, *this, Rgba::MAGENTA);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)convexPoly2DVerts.size(), &convexPoly2DVerts[0]);
}

// -----------------------------------------------------------------------
bool ConvexPoly2D::IsPointInside(Vec2 point)
{
	bool inside = true;
	for (const Line& edge : m_edges)
	{
		Vec2 center = (edge.lineEnd + edge.lineStart) / 2;
		if (DotProductVec2((point - center), edge.m_surfaceNormal) > 0.0f)
		{
			inside = false;
		}
	}

	return inside;
}
