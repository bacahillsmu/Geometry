#include "Game/Shapes/ConvexPoly.hpp"


// -----------------------------------------------------------------------
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"



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
	AddVertsForConvexPoly2D(convexPoly2DVerts, *this, Rgba::TEAL);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)convexPoly2DVerts.size(), &convexPoly2DVerts[0]);
}
