/*
 * QOpenGLRenderer.cpp
 *
 *  Created on: 04.04.2014
 *      Author: Patrik Huber
 */

#include "render/QOpenGLRenderer.hpp"

#include "render/Mesh.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <vector>

using std::vector;

namespace render {

QOpenGLRenderer::QOpenGLRenderer(QOpenGLContext* qOpenGlContext) : qOpenGlContext(qOpenGlContext), m_program(0)
{
	// this is initialize():
	m_program = new QOpenGLShaderProgram(qOpenGlContext);
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	m_program->link();
	m_posAttr = m_program->attributeLocation("posAttr");
	m_colAttr = m_program->attributeLocation("colAttr");
	m_texAttr = m_program->attributeLocation("texAttr");
	m_texWeightAttr = m_program->attributeLocation("texWeightAttr");
	m_matrixUniform = m_program->uniformLocation("matrix");

	glEnable(GL_TEXTURE_2D);
	//cv::Mat ocvimg = cv::imread("C:\\Users\\Patrik\\Documents\\GitHub\\img.png");
	cv::Mat ocvimg = cv::imread("C:\\Users\\Patrik\\Documents\\GitHub\\isoRegistered3D_square.png");
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, texture);
	cv::cvtColor(ocvimg, ocvimg, CV_BGR2RGB);
	cv::flip(ocvimg, ocvimg, 0); // Flip around the x-axis
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ocvimg.cols, ocvimg.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, ocvimg.ptr(0));

	std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Set nearest filtering mode for texture minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// Set bilinear filtering mode for texture magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Wrap texture coordinates by repeating
	// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// TODO: Put in d'tor:
	// glDeleteTextures(1, &texture);
}

void QOpenGLRenderer::render(render::Mesh mesh)
{
	//const qreal retinaScale = devicePixelRatio();
	glViewport(0, 0, viewportWidth * retinaScale, viewportHeight * retinaScale);

	//glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST); // to init
	// Enable back face culling
	glEnable(GL_CULL_FACE); // to init

	glEnable(GL_TEXTURE_2D);

	// Note: OpenGL: CCW triangles = front-facing.
	// Coord-axis: right = +x, up = +y, to back = -z, to front = +z

	m_program->bind();

	// Store the 3DMM vertices in GLfloat's
	// Todo: Measure time. Possible improvements: 1) See if Vec3f is in contiguous storage, if yes, maybe change my Vertex/Mesh structure.
	// 2) Maybe change the datatypes (see http://www.opengl.org/wiki/Vertex_Specification_Best_Practices 'Attribute sizes')
	vector<GLfloat> mmVertices;
	//render::Mesh mesh = morphableModel.getMean();
	mmVertices.clear();
	int numTriangles = mesh.tvi.size();
	for (int i = 0; i < numTriangles; ++i)
	{
		// First vertex x, y, z of the triangle
		const auto& triangle = mesh.tvi[i];
		mmVertices.push_back(mesh.vertex[triangle[0]].position[0]);
		mmVertices.push_back(mesh.vertex[triangle[0]].position[1]);
		mmVertices.push_back(mesh.vertex[triangle[0]].position[2]);
		// Second vertex x, y, z
		mmVertices.push_back(mesh.vertex[triangle[1]].position[0]);
		mmVertices.push_back(mesh.vertex[triangle[1]].position[1]);
		mmVertices.push_back(mesh.vertex[triangle[1]].position[2]);
		// Third vertex x, y, z
		mmVertices.push_back(mesh.vertex[triangle[2]].position[0]);
		mmVertices.push_back(mesh.vertex[triangle[2]].position[1]);
		mmVertices.push_back(mesh.vertex[triangle[2]].position[2]);
	}
	vector<GLfloat> mmColors;
	for (int i = 0; i < numTriangles; ++i)
	{
		// First vertex x, y, z of the triangle
		const auto& triangle = mesh.tci[i];
		mmColors.push_back(mesh.vertex[triangle[0]].color[0]);
		mmColors.push_back(mesh.vertex[triangle[0]].color[1]);
		mmColors.push_back(mesh.vertex[triangle[0]].color[2]);
		// Second vertex x, y, z
		mmColors.push_back(mesh.vertex[triangle[1]].color[0]);
		mmColors.push_back(mesh.vertex[triangle[1]].color[1]);
		mmColors.push_back(mesh.vertex[triangle[1]].color[2]);
		// Third vertex x, y, z
		mmColors.push_back(mesh.vertex[triangle[2]].color[0]);
		mmColors.push_back(mesh.vertex[triangle[2]].color[1]);
		mmColors.push_back(mesh.vertex[triangle[2]].color[2]);
	}
	vector<GLfloat> mmTex;
	for (int i = 0; i < numTriangles; ++i)
	{
		// First vertex u, v of the triangle
		const auto& triangle = mesh.tci[i]; // use tti?
		mmTex.push_back(mesh.vertex[triangle[0]].texcrd[0]);
		mmTex.push_back(mesh.vertex[triangle[0]].texcrd[1]);
		// Second vertex u, v
		mmTex.push_back(mesh.vertex[triangle[1]].texcrd[0]);
		mmTex.push_back(mesh.vertex[triangle[1]].texcrd[1]);
		// Third vertex u, v
		mmTex.push_back(mesh.vertex[triangle[2]].texcrd[0]);
		mmTex.push_back(mesh.vertex[triangle[2]].texcrd[1]);
	}

	float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	QMatrix4x4 matrix;
	matrix.ortho(-1.0f*aspect, 1.0f*aspect, -1.0f, 1.0f, 0.1f, 100.0f); // l r b t n f
	//matrix.ortho(-70.0f, 70.0f, -70.0f, 70.0f, 0.1f, 1000.0f);
	//matrix.perspective(60, aspect, 0.1, 100.0);
	matrix.translate(0, 0, -2);
	//matrix.rotate(30.0f, 1.0f, 0.0f, 0.0f);
	//matrix.rotate(50.0f, 0.0f, 1.0f, 0.0f);
	//matrix.scale(0.009f);
	matrix.scale(0.3f);

	m_program->setUniformValue(m_matrixUniform, matrix);

	glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, &mmVertices[0]); // vertices
	glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, &mmColors[0]); // colors
	glVertexAttribPointer(m_texAttr, 2, GL_FLOAT, GL_FALSE, 0, &mmTex[0]); // texCoords
	m_program->setAttributeValue(m_texWeightAttr, 0.4f);
	m_program->setUniformValue("texture", texture);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, texture);

	glEnableVertexAttribArray(m_posAttr);
	glEnableVertexAttribArray(m_colAttr);
	glEnableVertexAttribArray(m_texAttr);

	glDrawArrays(GL_TRIANGLES, 0, numTriangles * 3); // 6; (2 triangles) how many vertices to render

	glDisableVertexAttribArray(m_texAttr);
	glDisableVertexAttribArray(m_colAttr);
	glDisableVertexAttribArray(m_posAttr);

	m_program->release();

	cv::Mat framebuffer = cv::imread("C:\\Users\\Patrik\\Documents\\GitHub\\box_screenbuffer10.png");
	//Mat textureMap = extractTexture(mesh, matrix, width(), height(), framebuffer);
	//cv::imwrite("C:\\Users\\Patrik\\Documents\\GitHub\\img_extracted10.png", textureMap);
	++m_frame;
}

} /* namespace render */
