#include "UIExtensions.h"
#include "HelmsleyVolume.h"
#include "cvrKernel/NodeMask.h"

#include <algorithm>

using namespace cvr;

osg::Program* ColorPickerSaturationValue::_svprogram = nullptr;
osg::Program* ColorPickerHue::_hueprogram = nullptr;
osg::Program* Tent::_triangleProg = nullptr;


#pragma region VisibilityToggle
VisibilityToggle::VisibilityToggle(std::string text)
	: UIToggle()
{
	std::string dir = ConfigManager::getEntry("Plugin.HelmsleyVolume.ImageDir");
	eye = new UICheckbox(dir + "eye_closed.png", dir + "eye_open.png");
	eye->offElement->setTransparent(true);
	eye->offElement->setColor(osg::Vec4(0,0,0,1));
	eye->onElement->setTransparent(true);
	eye->onElement->setColor(osg::Vec4(0, 0, 0, 1));
	eye->setAspect(osg::Vec3(1, 0, 1));
	eye->setPercentSize(osg::Vec3(0.3, 1, 0.9));
	eye->setPercentPos(osg::Vec3(0.05, 0, -0.05));
	eye->setDisplayed(0);
	eye->setAlign(UIElement::CENTER_CENTER);
	addChild(eye);

	label = new UIText(text, 50.0f, osgText::TextBase::LEFT_CENTER);
	label->setPercentSize(osg::Vec3(0.6, 1, 1));
	label->setPercentPos(osg::Vec3(0.4, 0, 0));
	addChild(label);
}

bool VisibilityToggle::onToggle()
{
	std::cerr << "TOGGLE " << _on << std::endl;
	if (_on)
	{
		eye->setDisplayed(1);
	}
	else
	{
		eye->setDisplayed(0);
	}

	if (_callback)
	{
		_callback->uiCallback(this);
	}
	return true;
}

#pragma endregion

#pragma region CallbackToggle
bool CallbackToggle::onToggle()
{
	if (_callback)
	{
		_callback->uiCallback(this);
	}
	return true;
}
#pragma endregion

#pragma region CallbackRadial
void CallbackRadial::onSelectionChange()
{
	if (_callback)
	{
		_callback->uiCallback(this);
	}
}
#pragma endregion

#pragma region ToolRadialButton
ToolRadialButton::ToolRadialButton(UIRadial* parent, std::string iconpath)
	: UIRadialButton(parent)
{
	_icon = new UITexture(iconpath);
	_icon->setTransparent(true);
	_icon->setColor(osg::Vec4(0, 0, 0, 1));
	_icon->setPercentSize(osg::Vec3(0.8, 1, 0.8));
	_icon->setPercentPos(osg::Vec3(0.1, 0, -0.1));

	_quad = new UIQuadElement(osg::Vec4(0.95, 0.95, 0.95, 1));
	_quad->setRounding(0.0f, 0.05f);
	_quad->setTransparent(true);
	_quad->addChild(_icon);
	addChild(_quad);
}

void ToolRadialButton::processHover(bool enter)
{
	if (enter)
	{
		_quad->setColor(osg::Vec4(0.8, 0.8, 0.8, 1));
	}
	else
	{
		_quad->setColor(osg::Vec4(0.95, 0.95, 0.95, 1.0));
	}
}
#pragma endregion

#pragma region ToolToggle
ToolToggle::ToolToggle(std::string iconpath)
	: CallbackToggle()
{
	_icon = new UITexture(iconpath);
	_icon->setTransparent(true);
	_icon->setColor(osg::Vec4(0, 0, 0, 1));
	_icon->setPercentSize(osg::Vec3(0.8, 1, 0.8));
	_icon->setPercentPos(osg::Vec3(0.1, 0, -0.1));

	_quad = new UIQuadElement(osg::Vec4(0.95, 0.95, 0.95, 1));
	_quad->setRounding(0.0f, 0.05f);
	_quad->setTransparent(true);
	_quad->addChild(_icon);
	addChild(_quad);
}

void ToolToggle::processHover(bool enter)
{
	if (enter)
	{
		_quad->setColor(osg::Vec4(0.8, 0.8, 0.8, 1));
	}
	else
	{
		_quad->setColor(osg::Vec4(0.95, 0.95, 0.95, 1.0));
	}
}
#pragma endregion

#pragma region CallbackSlider
void CallbackSlider::setMax(float max)
{
	_max = max;
}

void CallbackSlider::setMin(float min)
{
	_min = min;
}

float CallbackSlider::getMax()
{
	return _max;
}

float CallbackSlider::getMin()
{
	return _min;
}

bool CallbackSlider::onPercentChange()
{
	if (_callback)
	{
		_callback->uiCallback(this);
		return true;
	}
	return false;
}
#pragma endregion

#pragma region ShaderQuad
void ShaderQuad::updateGeometry()
{
	UIQuadElement::updateGeometry();

	if (_program.valid())
	{
		_geode->getDrawable(0)->getOrCreateStateSet()->setAttributeAndModes(_program.get(), osg::StateAttribute::ON);
	}
}

template <typename T>
void ShaderQuad::addUniform(std::string uniform, T initialvalue)
{
	_uniforms[uniform] = new osg::Uniform(uniform.c_str(), initialvalue);
	_geode->getOrCreateStateSet()->addUniform(_uniforms[uniform]);
}

void ShaderQuad::addUniform(std::string uniform)
{
	_uniforms[uniform] = new osg::Uniform(uniform.c_str(), 0.0f);
	_geode->getOrCreateStateSet()->addUniform(_uniforms[uniform]);
}

void ShaderQuad::addUniform(osg::Uniform* uniform)
{
	_uniforms[uniform->getName()] = uniform;
	_geode->getOrCreateStateSet()->addUniform(uniform);
}

osg::Uniform* ShaderQuad::getUniform(std::string uniform)
{
	return _uniforms[uniform];
}

void ShaderQuad::setShaderDefine(std::string name, std::string define, osg::StateAttribute::Values on)
{
	_geode->getOrCreateStateSet()->setDefine(name, define, on);
}
#pragma endregion

#pragma region PlanePointer
void PlanePointer::setPointer(float x, float y)
{
	osg::Vec2 pos = osg::Vec2(x, y);
	if (_pointer != pos)
	{
		_pointer = pos;
		_dirty = true;
	}
}

void PlanePointer::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();
	//_geode = new osg::Geode();

	_group->addChild(_transform);
	_transform->addChild(_intersect);
	//_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);

	osg::Geometry* drawable = UIUtil::makeQuad(1, 1, osg::Vec4(1, 1, 1, 1), osg::Vec3(0, 0, 0));
	//_geode->addDrawable(drawable);
	//should quadelement have an intersectable?
	_intersect->addDrawable(drawable);

	updateGeometry();

}

void PlanePointer::updateGeometry()
{
	osg::Matrix mat = osg::Matrix();
	mat.makeScale(_actualSize);
	mat.postMultTranslate(_actualPos);
	_transform->setMatrix(mat);
}

bool PlanePointer::processEvent(InteractionEvent* event)
{
	TrackedButtonInteractionEvent* tie = event->asTrackedButtonEvent();
	if (tie && tie->getButton() == _button)
	{
		if (tie->getInteraction() == BUTTON_DOWN)
		{
			_held = true;
			return true;
		}
		else if (tie->getInteraction() == BUTTON_DRAG && _held)
		{
			osg::MatrixList ltw = _intersect->getWorldMatrices();
			osg::Matrix m = ltw[0];//osg::Matrix::identity();
			//for (int i = 0; i < ltw.size(); ++i)
			//{
			//	m.postMult(ltw[i]);
			//}
			osg::Matrix mi = osg::Matrix::inverse(m);

			osg::Vec4d l4 = osg::Vec4(0, 1, 0, 0) * TrackingManager::instance()->getHandMat(tie->getHand());
			osg::Vec3 l = osg::Vec3(l4.x(), l4.y(), l4.z());

			osg::Vec4d l04 = osg::Vec4(0, 0, 0, 1) * TrackingManager::instance()->getHandMat(tie->getHand());
			osg::Vec3 l0 = osg::Vec3(l04.x(), l04.y(), l04.z());

			osg::Vec4d n4 = osg::Vec4(0, 1, 0, 0) * m;
			osg::Vec3 n = osg::Vec3(n4.x(), n4.y(), n4.z());

			osg::Vec4d p04 = osg::Vec4(0, 0, 0, 1) * m;
			osg::Vec3 p0 = osg::Vec3(p04.x(), p04.y(), p04.z());


			osg::Vec3 p = l0 + l * (((p0 - l0) * n) / (l * n));

			osg::Vec4 pl = osg::Vec4(p.x(), p.y(), p.z(), 1) * mi;

			setPointer(pl.x(), pl.z());
			//_percent = pl.x(); // _lastHitPoint.x();
			//_dirty = true;
			//std::cerr << "<" << _lastHitPoint.x() << ", " << _lastHitPoint.y() << ", " << _lastHitPoint.z() << ">" << std::endl;
			return onPosChange();
		}
		else if (tie->getInteraction() == BUTTON_UP)
		{
			_held = false;
			return false;
		}
	}

	return false;
}
#pragma endregion

#pragma region ColorPickerSV
ColorPickerSaturationValue::ColorPickerSaturationValue()
	: PlanePointer()
{
	setPointer(1, 1);
	_sv = osg::Vec2(1, 1);
	_shader = new ShaderQuad();
	_shader->setProgram(getOrLoadProgram());
	_shader->addUniform("Hue");
	_shader->getUniform("Hue")->set(0.0f);
	addChild(_shader);

	_indicator = new UIQuadElement();
	_indicator->setSize(osg::Vec3(0, 1, 0), osg::Vec3(20, 0, 20));
	_indicator->setAbsolutePos(osg::Vec3(-10, -0.2f, 10));
	addChild(_indicator);
}

bool ColorPickerSaturationValue::onPosChange()
{
	_sv = _pointer;
	_sv.x() = std::min(std::max(0.0f, _sv.x()), 1.0f);
	_sv.y() = 1.0f - std::min(std::max(0.0f, -_sv.y()), 1.0f);
	_indicator->setPercentPos(osg::Vec3(_sv.x(), 0, -1.0f +_sv.y()));
	if (_callback)
	{
		_callback->uiCallback(this);
	}
	return true;
}

void ColorPickerSaturationValue::setHue(float hue)
{
	if (_hue != hue)
	{
		_hue = hue;
		std::cout << "Hue: " << osg::Uniform::getTypename(_shader->getUniform("Hue")->getType()) << std::endl;
		_shader->getUniform("Hue")->set(_hue);
	}
}

void ColorPickerSaturationValue::set_indicator() {
	_indicator->setPercentPos(osg::Vec3(_sv.x(), 0, -1.0f + _sv.y()));
}

osg::Program* ColorPickerSaturationValue::getOrLoadProgram()
{
	if (!_svprogram)
	{
		const std::string vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
		const std::string frag = HelmsleyVolume::loadShaderFile("colorpickerSV.frag");
		_svprogram = new osg::Program;
		_svprogram->setName("ColorpickerSV");
		_svprogram->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
		_svprogram->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	}

	return _svprogram;
}
#pragma endregion

#pragma region ColorPickerH
ColorPickerHue::ColorPickerHue()
	: PlanePointer()
{
	setPointer(0, 0);
	_hue = 0;
	_shader = new ShaderQuad();
	_shader->setProgram(getOrLoadProgram());
	_shader->addUniform("SV", osg::Vec2(1.0f, 1.0f));
	addChild(_shader);

	_indicator = new UIQuadElement();
	_indicator->setSize(osg::Vec3(1, 1, 0), osg::Vec3(0, 0, 20));
	_indicator->setAbsolutePos(osg::Vec3(0, -0.2f, 10));
	addChild(_indicator);
}

bool ColorPickerHue::onPosChange()
{
	_hue = 1.0f - std::min(std::max(0.0f, -_pointer.y()), 1.0f);
	_indicator->setPercentPos(osg::Vec3(0, 0, -1.0f + _hue));

	//changebox
	

	//
	if (_callback)
	{
		_callback->uiCallback(this);
	}
	return true;
}

void ColorPickerHue::setSV(osg::Vec2 SV)
{
	if (_sv != SV)
	{
		_sv = SV;
		std::cout << "SV: " << osg::Uniform::getTypename(_shader->getUniform("SV")->getType()) << std::endl;
		_shader->getUniform("SV")->set(_sv);
	}
}

void ColorPickerHue::set_indicator() {
	_hue = 1.0f - std::min(std::max(0.0f, -_pointer.y()), 1.0f);
	_indicator->setPercentPos(osg::Vec3(0, 0, -1.0f + _hue));
}

osg::Program* ColorPickerHue::getOrLoadProgram()
{
	if (!_hueprogram)
	{
		const std::string vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
		const std::string frag = HelmsleyVolume::loadShaderFile("colorpickerH.frag");
		_hueprogram = new osg::Program;
		_hueprogram->setName("ColorpickerH");
		_hueprogram->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
		_hueprogram->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	}

	return _hueprogram;
}
#pragma endregion


TentWindow::TentWindow() :
	UIElement()
{
	_bknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	addChild(_bknd);

	_tent = new Tent(osg::Vec4(1.0,0.0,0.0,1.0));
	_tent->setPercentPos(osg::Vec3(.7, -0.1, -0.5));
	_tent->setPercentSize(osg::Vec3( 2, 0.015, 1));

	_bknd->addChild(_tent);

	_bottomWidth = new CallbackSlider();
	_bottomWidth->setPercentPos(osg::Vec3(0.025, 0, -0.60));
	_bottomWidth->setPercentSize(osg::Vec3(0.9, 1, 0.2));
	_bottomWidth->handle->setAbsoluteSize(osg::Vec3(20, 0, 0));
	_bottomWidth->handle->setAbsolutePos(osg::Vec3(-10, -0.2f, 0));
	_bottomWidth->handle->setPercentSize(osg::Vec3(0, 1, 1));
	_bottomWidth->setMax(1.0f);
	_bottomWidth->setMin(0.001f);
	_bottomWidth->setCallback(this);
	_bottomWidth->setPercent(.5);

	_centerPos = new CallbackSlider();
	_centerPos->setPercentPos(osg::Vec3(0.025, 0, -0.80));
	_centerPos->setPercentSize(osg::Vec3(0.9, 1, 0.2));
	_centerPos->handle->setAbsoluteSize(osg::Vec3(20, 0, 0));
	_centerPos->handle->setAbsolutePos(osg::Vec3(-10, -0.2f, 0));
	_centerPos->handle->setPercentSize(osg::Vec3(0, 1, 1));
	_centerPos->setMax(1.0f);
	_centerPos->setMin(0.001f);
	_centerPos->setCallback(this);
	_centerPos->setPercent(.7);
	
	
	
	_bknd->addChild(_bottomWidth);
	_bknd->addChild(_centerPos);
	
}

void TentWindow::uiCallback(UICallbackCaller* ui) {
	if (ui == _bottomWidth) {
		std::cout << "Width: " << _bottomWidth->getAdjustedValue() << std::endl;
		_tent->setPercentSize(osg::Vec3(_bottomWidth->getAdjustedValue()*4, 0.015, 1));
		_volume->_computeUniforms["OpacityWidth"]->set(_bottomWidth->getAdjustedValue()*2);
		_tent->addUniform("Width", _bottomWidth->getAdjustedValue()*2);
		
	}
	if (ui == _centerPos) {
		std::cout << "Center: " << _centerPos->getAdjustedValue() << std::endl;
		_tent->setPercentPos(osg::Vec3(_centerPos->getAdjustedValue(), -0.1, -0.5));
		_volume->_computeUniforms["OpacityCenter"]->set(_centerPos->getAdjustedValue());
		_tent->addUniform("Center", _centerPos->getAdjustedValue());
	}
	_volume->setDirtyAll();
	
}

void Tent::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();

	_group->addChild(_transform);
	_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);
	osg::Geometry* polyGeom = new osg::Geometry();

	
	osg::Vec3 myCoords[] =
	{
		osg::Vec3(rightPointX, 0.0, 0.0),
		osg::Vec3(0.0, 0.0, 0.5),
		osg::Vec3(leftPointX, 0.0, 0.0)

	};
	int numCoords = sizeof(myCoords) / sizeof(osg::Vec3);

	osg::Vec3Array* vertices = new osg::Vec3Array(numCoords, myCoords);

	
	polyGeom->setVertexArray(vertices);

	polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 3));

	_geode->addDrawable(polyGeom);


	osg::Vec2Array* texcoords = new osg::Vec2Array;
	texcoords->push_back(osg::Vec2(1, 0));
	texcoords->push_back(osg::Vec2(0.5, 1));
	texcoords->push_back(osg::Vec2(0, 0));

	polyGeom->setVertexAttribArray(1, texcoords, osg::Array::BIND_PER_VERTEX);
	setTransparent(true);

	updateGeometry();
}

void Tent::updateGeometry()
{
	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(_color);
	((osg::Geometry*)_geode->getDrawable(0))->setColorArray(colors, osg::Array::BIND_OVERALL);
	((osg::Geometry*)_geode->getDrawable(0))->setVertexAttribArray(2, colors, osg::Array::BIND_OVERALL);

	osg::Matrix mat = osg::Matrix();
	mat.makeScale(_actualSize);
	mat.postMultTranslate(_actualPos);
	_transform->setMatrix(mat);


	if (_program.valid())
	{
		_geode->getDrawable(0)->getOrCreateStateSet()->setAttributeAndModes(_program.get(), osg::StateAttribute::ON);

	}
}

void Tent::setColor(osg::Vec4 color)
{
	if (_color != color)
	{
		_color = color;
		_dirty = true;
	}
}

void Tent::setTransparent(bool transparent)
{
	if (transparent)
	{
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	}
	else
	{
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::OFF);
	}
}

void Tent::setRounding(float absRounding, float percentRounding)
{
	_absoluteRounding->set(absRounding);
	_percentRounding->set(percentRounding);
}


template <typename T>
void Tent::addUniform(std::string uniform, T initialvalue)
{
	_uniforms[uniform] = new osg::Uniform(uniform.c_str(), initialvalue);
	_geode->getOrCreateStateSet()->addUniform(_uniforms[uniform]);
}

void Tent::addUniform(std::string uniform)
{
	_uniforms[uniform] = new osg::Uniform(uniform.c_str(), 0.0f);
	_geode->getOrCreateStateSet()->addUniform(_uniforms[uniform]);
}

void Tent::addUniform(osg::Uniform* uniform)
{
	_uniforms[uniform->getName()] = uniform;
	_geode->getOrCreateStateSet()->addUniform(uniform);
}

osg::Uniform* Tent::getUniform(std::string uniform)
{
	return _uniforms[uniform];
}

void Tent::setShaderDefine(std::string name, std::string define, osg::StateAttribute::Values on)
{
	_geode->getOrCreateStateSet()->setDefine(name, define, on);
}

osg::Program* Tent::getOrLoadProgram()
{
	if (!_triangleProg)
	{
		const std::string vert = HelmsleyVolume::loadShaderFile("transferFunction.vert");
		const std::string frag = HelmsleyVolume::loadShaderFile("triangle.frag");
		_triangleProg = new osg::Program;
		_triangleProg->setName("Triangle");
		_triangleProg->addShader(new osg::Shader(osg::Shader::VERTEX, vert));
		_triangleProg->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag));
	}

	return _triangleProg;
}



#pragma region ColorPicker

ColorPicker::ColorPicker() :
	UIElement()
{
	color = osg::Vec3(0, 1, 1);

	_bknd = new UIQuadElement(UI_BACKGROUND_COLOR);
	addChild(_bknd);
	
	float border = 20;

	_hue = new ColorPickerHue();
	_hue->setPos(osg::Vec3(0.8f, 0.0f, 0.0f), osg::Vec3(border, -0.1f, -border));
	_hue->setSize(osg::Vec3(0.2f, 1.0f, 1.0f), osg::Vec3(-border*2, 0, -border * 2));
	_bknd->addChild(_hue);
	_hue->setCallback(this);

	_sv = new ColorPickerSaturationValue();
	_sv->setAbsolutePos(osg::Vec3(border, -0.1f, -border));
	_sv->setSize(osg::Vec3(0.8f, 1.0f, 1.0f), osg::Vec3(-border * 2, 0, -border * 2));
	_bknd->addChild(_sv);
	_sv->setCallback(this);
	

}

void ColorPicker::uiCallback(UICallbackCaller* ui)
{
	if (ui == _hue)
	{
		float h = _hue->getHue();
		if (color.x() != h)
		{
			color.x() = h;
			if (_callback)
			{
				_callback->uiCallback(this);
			}
		}
		_sv->setHue(h);
	}
	else if(ui == _sv)
	{
		osg::Vec2 sv = _sv->getSV();
		if (color.y() != sv.x() || color.z() != sv.y())
		{
			color.y() = sv.x();
			color.z() = sv.y();
			if (_callback)
			{
				_callback->uiCallback(this);
			}
		}

		_hue->setSV(sv);
	}

	//GA
	*_saveColor = color;
	osg::Vec3 solidCol = ColorPicker::returnColor();
	
	_target->setColor(osg::Vec4(solidCol, 1.0));
	_transferFunction = "vec3(" + std::to_string(solidCol.x()) + "," + std::to_string(solidCol.y()) + "," + std::to_string(solidCol.z()) + ");";

	if (_radial->getCurrent() == 2) {
		switch (_organRGB) {
			case BLADDER: 
				_volume->getCompute()->getOrCreateStateSet()->setDefine("BLADDER_RGB", _transferFunction, osg::StateAttribute::ON);
				break;
			case COLON:
				_volume->getCompute()->getOrCreateStateSet()->setDefine("COLON_RGB", _transferFunction, osg::StateAttribute::ON);
				break;
			case SPLEEN:
				_volume->getCompute()->getOrCreateStateSet()->setDefine("SPLEEN_RGB", _transferFunction, osg::StateAttribute::ON);
				break;
			case KIDNEY:
				_volume->getCompute()->getOrCreateStateSet()->setDefine("KIDNEY_RGB", _transferFunction, osg::StateAttribute::ON); 
				break;
			case BODY:
				_volume->getCompute()->getOrCreateStateSet()->setDefine("BODY_RGB", _transferFunction, osg::StateAttribute::ON);
				break;
		}

		_colorDisplay->setShaderDefine("COLOR_FUNCTION", _transferFunction, osg::StateAttribute::ON);
		_volume->setDirtyAll();
	}
	
	
	//GA

	std::cerr << "Color: <" << color.x() << ", " << color.y() << ", " << color.z() << ">" << std::endl;
	std::cerr << "ColorRGB: <" << rgbColor.x() << ", " << rgbColor.y() << ", " << rgbColor.z() << ">" << std::endl;
}

void ColorPicker::setButton(cvr::UIQuadElement* target) {
	_target = target;

}

osg::Vec3 ColorPicker::returnColor() {
	//hsv to rgb
	double H = color.x() * 360.0;
	double S = color.y();
	double V = color.z();

	double C = S * V;
	double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	double m = V - C;
	double Rs, Gs, Bs;

	if (H >= 0 && H < 60 ) {
		Rs = C;
		Gs = X;
		Bs = 0;
	}
	else if (H >= 60  && H < 120 ) {
		Rs = X;
		Gs = C;
		Bs = 0;
	}
	else if (H >= 120  && H < 180 ) {
		Rs = 0;
		Gs = C;
		Bs = X;
	}
	else if (H >= 180 && H < 240 ) {
		Rs = 0;
		Gs = X;
		Bs = C;
	}
	else if (H >= 240  && H < 300 ) {
		Rs = X;
		Gs = 0;
		Bs = C;
	}
	else {
		Rs = C;
		Gs = 0;
		Bs = X;
	}

	rgbColor.x() = (Rs + m);
	rgbColor.y() = (Gs + m);
	rgbColor.z() = (Bs + m);
	return rgbColor;
}
#pragma endregion