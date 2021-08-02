#if defined(USE_OPENGLES_10)

#include "renderers/Renderer.h"
#include "Log.h"
#include "Settings.h"
#include "math/Transform4x4f.h"

#include <GLES/gl.h>
#include <SDL.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

#include <go2/display.h>
#include <go2/input.h>
#include <go2/audio.h>
#include <drm/drm_fourcc.h>
#include "BatteryIcons.h"
#include "VolumeIcons.h"
#include "WifiIcons.h"
#include "BrightnessIcon.h"
#include "PowerIcon.h"
#include "BlankIcon.h"
#include "ArkOS.h"


static go2_input_t* input = nullptr;
static go2_surface_t* titlebarSurface = nullptr;
static unsigned int frame = 0;

namespace Renderer
{
	//static SDL_GLContext sdlContext = nullptr;

	static go2_context_t* context = nullptr;
	static go2_presenter_t* presenter = nullptr;

	static GLenum convertBlendFactor(const Blend::Factor _blendFactor)
	{
		switch(_blendFactor)
		{
			case Blend::ZERO:                { return GL_ZERO;                } break;
			case Blend::ONE:                 { return GL_ONE;                 } break;
			case Blend::SRC_COLOR:           { return GL_SRC_COLOR;           } break;
			case Blend::ONE_MINUS_SRC_COLOR: { return GL_ONE_MINUS_SRC_COLOR; } break;
			case Blend::SRC_ALPHA:           { return GL_SRC_ALPHA;           } break;
			case Blend::ONE_MINUS_SRC_ALPHA: { return GL_ONE_MINUS_SRC_ALPHA; } break;
			case Blend::DST_COLOR:           { return GL_DST_COLOR;           } break;
			case Blend::ONE_MINUS_DST_COLOR: { return GL_ONE_MINUS_DST_COLOR; } break;
			case Blend::DST_ALPHA:           { return GL_DST_ALPHA;           } break;
			case Blend::ONE_MINUS_DST_ALPHA: { return GL_ONE_MINUS_DST_ALPHA; } break;
			default:                         { return GL_ZERO;                }
		}

	} // convertBlendFactor

	static GLenum convertTextureType(const Texture::Type _type)
	{
		switch(_type)
		{
			case Texture::RGBA:  { return GL_RGBA;  } break;
			case Texture::ALPHA: { return GL_ALPHA; } break;
			default:             { return GL_ZERO;  }
		}

	} // convertTextureType

	unsigned int convertColor(const unsigned int _color)
	{
		// convert from rgba to abgr
		unsigned char r = ((_color & 0xff000000) >> 24) & 255;
		unsigned char g = ((_color & 0x00ff0000) >> 16) & 255;
		unsigned char b = ((_color & 0x0000ff00) >>  8) & 255;
		unsigned char a = ((_color & 0x000000ff)      ) & 255;

		return ((a << 24) | (b << 16) | (g << 8) | (r));

	} // convertColor

	unsigned int getWindowFlags()
	{
		return SDL_WINDOW_OPENGL;

	} // getWindowFlags

	void setupWindow()
	{
	} // setupWindow

	void createContext()
	{
		// sdlContext = SDL_GL_CreateContext(getSDLWindow());
		// SDL_GL_MakeCurrent(getSDLWindow(), sdlContext);
		input = go2_input_create();

		go2_context_attributes_t attr;
		attr.major = 1;
		attr.minor = 0;
		attr.red_bits = 8;
		attr.green_bits = 8;
		attr.blue_bits = 8;
		attr.alpha_bits = 8;
		attr.depth_bits = 24;
		attr.stencil_bits = 0;

		go2_display_t* display = getDisplay();
		int w = go2_display_height_get(display);
		int h = go2_display_width_get(display);

		if (!Renderer::isFullScreenMode())
			titlebarSurface = go2_surface_create(display, w, 16, DRM_FORMAT_RGB565);

		context = go2_context_create(display, w, h, &attr);
		go2_context_make_current(context);

		presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff080808);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		std::string glExts = (const char*)glGetString(GL_EXTENSIONS);
		LOG(LogInfo) << "Renderer_GLES10::createContext() - Checking available OpenGL extensions...";
		LOG(LogInfo) << "Renderer_GLES10::createContext() - ARB_texture_non_power_of_two: " << (glExts.find("ARB_texture_non_power_of_two") != std::string::npos ? "ok" : "MISSING");

	} // createContext

	void destroyContext()
	{
		//SDL_GL_DeleteContext(sdlContext);
		//sdlContext = nullptr;
		go2_context_destroy(context);
		context = nullptr;

		go2_presenter_destroy(presenter);
		presenter = nullptr;

		if (!Renderer::isFullScreenMode())
		{
			go2_surface_destroy(titlebarSurface);
			titlebarSurface = nullptr;
		}

		go2_input_destroy(input);
		input = nullptr;
	} // destroyContext

	unsigned int createTexture(const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data)
	{
		const GLenum type = convertTextureType(_type);
		unsigned int texture;

		glGenTextures(1, &texture);
		bindTexture(texture);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _linear ? GL_LINEAR : GL_NEAREST);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, type, _width, _height, 0, type, GL_UNSIGNED_BYTE, _data);

		return texture;

	} // createTexture

	void destroyTexture(const unsigned int _texture)
	{
		glDeleteTextures(1, &_texture);

	} // destroyTexture

	void updateTexture(const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data)
	{
		bindTexture(_texture);

		if (_x == -1 && _y == -1)
		{
			const GLenum type = convertTextureType(_type);
			glTexImage2D(GL_TEXTURE_2D, 0, type, _width, _height, 0, type, GL_UNSIGNED_BYTE, _data);
		}
		else
			glTexSubImage2D(GL_TEXTURE_2D, 0, _x, _y, _width, _height, convertTextureType(_type), GL_UNSIGNED_BYTE, _data);

		bindTexture(0);

	} // updateTexture

	void bindTexture(const unsigned int _texture)
	{
		glBindTexture(GL_TEXTURE_2D, _texture);

		if(_texture == 0) glDisable(GL_TEXTURE_2D);
		else              glEnable(GL_TEXTURE_2D);

	} // bindTexture

	void drawLines(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(  2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].tex);
		glColorPointer(   4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col);

		glDrawArrays(GL_LINES, 0, _numVertices);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_BLEND);

	} // drawLines

	void drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(  2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].tex);
		glColorPointer(   4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, _numVertices);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_BLEND);

	} // drawTriangleStrips

	void setProjection(const Transform4x4f& _projection)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf((GLfloat*)&_projection);

	} // setProjection

	void setMatrix(const Transform4x4f& _matrix)
	{
		Transform4x4f matrix = _matrix;
		matrix.round();
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((GLfloat*)&matrix);

	} // setMatrix

	void setViewport(const Rect& _viewport)
	{
		// glViewport starts at the bottom left of the window
		glViewport( _viewport.x, getWindowHeight() - _viewport.y - _viewport.h, _viewport.w, _viewport.h);

	} // setViewport

	void setScissor(const Rect& _scissor)
	{
		if((_scissor.x == 0) && (_scissor.y == 0) && (_scissor.w == 0) && (_scissor.h == 0))
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			// glScissor starts at the bottom left of the window
			glScissor(_scissor.x, getWindowHeight() - _scissor.y - _scissor.h, _scissor.w, _scissor.h);
			glEnable(GL_SCISSOR_TEST);
		}

	} // setScissor

	void setSwapInterval()
	{
	} // setSwapInterval

	void swapBuffers()
	{
		//SDL_GL_SwapWindow(getSDLWindow());

		if (context)
		{
			go2_display_t* display = getDisplay();
			int w = go2_display_height_get(display);
			int h = go2_display_width_get(display);

			if (!Renderer::isFullScreenMode())
			{
				{
					// Battery level
					const uint8_t* src = battery_image.pixel_data;
					int src_stride = 32 * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					int batcapacity = 0;
					int fd;
					char buffer[10];
					fd = open("/sys/class/power_supply/battery/capacity", O_RDONLY);
					if (fd > 0)
					{
						memset(buffer, 0, 10);
						ssize_t count = read(fd, buffer, 10);
						if( count > 0 )
						{
							batcapacity = atoi(buffer);
						}
						close(fd);
					}

					int batteryIndex;
					if (batcapacity == 1)
					{
						batteryIndex = 0;
					}
					else if (batcapacity <= 5)
					{
						batteryIndex = 1;
					}
					else if (batcapacity <= 10)
					{
						batteryIndex = 2;
					}
					else if (batcapacity <= 15)
					{
						batteryIndex = 3;
					}
					else if (batcapacity <= 20)
					{
						batteryIndex = 4;
					}
					else if (batcapacity <= 25)
					{
						batteryIndex = 5;
					}
					else if (batcapacity <= 30)
					{
						batteryIndex = 6;
					}
					else if (batcapacity <= 35)
					{
						batteryIndex = 7;
					}
					else if (batcapacity <= 40)
					{
						batteryIndex = 8;
					}
					else if (batcapacity <= 45)
					{
						batteryIndex = 9;
					}
					else if (batcapacity <= 50)
					{
						batteryIndex = 10;
					}
					else if (batcapacity <= 55)
					{
						batteryIndex = 11;
					}
					else if (batcapacity <= 60)
					{
						batteryIndex = 12;
					}
					else if (batcapacity <= 65)
					{
						batteryIndex = 13;
					}
					else if (batcapacity <= 70)
					{
						batteryIndex = 14;
					}
					else if (batcapacity <= 75)
					{
						batteryIndex = 15;
					}
					else if (batcapacity <= 80)
					{
						batteryIndex = 16;
					}
					else if (batcapacity <= 85)
					{
						batteryIndex = 17;
					}
					else if (batcapacity <= 90)
					{
						batteryIndex = 18;
					}
					else if (batcapacity <= 95)
					{
						batteryIndex = 19;
					}
					else if (batcapacity == 100)
					{
						batteryIndex = 20;
					}
					else
					{
						batteryIndex = 20;
					}

					src += (batteryIndex * 16 * src_stride);
					dst += (w - 32) * sizeof(short);

					for (int y = 0; y < 16; ++y)
					{
						memcpy(dst, src, 32 * sizeof(short));

						src += src_stride;
						dst += dst_stride;
					}
				} // Battery level END

				{
					// Volume level
					const uint8_t* src = volume_image.pixel_data;
					int src_stride = 32 * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					uint32_t volume = go2_audio_volume_get(NULL);

					int volumeIndex;
					if (volume == 0)
					{
						volumeIndex = 0;
					}
					else if (volume <= 5)
					{
						volumeIndex = 1;
					}
					else if (volume <= 10)
					{
						volumeIndex = 2;
					}
					else if (volume <= 15)
					{
						volumeIndex = 3;
					}
					else if (volume <= 20)
					{
						volumeIndex = 4;
					}
					else if (volume <= 25)
					{
						volumeIndex = 5;
					}
					else if (volume <= 30)
					{
						volumeIndex = 6;
					}
					else if (volume <= 35)
					{
						volumeIndex = 7;
					}
					else if (volume <= 40)
					{
						volumeIndex = 8;
					}
					else if (volume <= 45)
					{
						volumeIndex = 9;
					}
					else if (volume <= 50)
					{
						volumeIndex = 10;
					}
					else if (volume <= 55)
					{
						volumeIndex = 11;
					}
					else if (volume <= 60)
					{
						volumeIndex = 12;
					}
					else if (volume <= 65)
					{
						volumeIndex = 13;
					}
					else if (volume <= 70)
					{
						volumeIndex = 14;
					}
					else if (volume <= 75)
					{
						volumeIndex = 15;
					}
					else if (volume <= 80)
					{
						volumeIndex = 16;
					}
					else if (volume <= 85)
					{
						volumeIndex = 17;
					}
					else if (volume <= 90)
					{
						volumeIndex = 18;
					}
					else if (volume <= 95)
					{
						volumeIndex = 19;
					}
					else if (volume = 100)
					{
						volumeIndex = 20;
					}
					else
					{
						volumeIndex = 20;
					}

					src += (volumeIndex * 16 * src_stride);
					//dst += (480 - 32) * sizeof(short);

					for (int y = 0; y < 16; ++y)
					{
						memcpy(dst, src, 32 * sizeof(short));

						src += src_stride;
						dst += dst_stride;
					}
				} // Volume level END
				{
					// Title
					const uint8_t* src = header.pixel_data;
					int src_stride = header.width * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					dst += ((w / 2) - (header.width / 2)) * sizeof(short);

					for (int y = 0; y < 16; ++y)
					{
						memcpy(dst, src, src_stride);

						src += src_stride;
						dst += dst_stride;
					}
				} // Title END
				{
					// Brightness level
					const uint8_t* src = brightness_image.pixel_data;
					int src_stride = 32 * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					int brightnessIndex = 0;
					int brightness = 50;

					try {
						brightness = (int) go2_display_backlight_get(NULL);
					} catch (...) {}

					if (brightness == 0)
					{
						brightnessIndex = 0;
					}
					else if (brightness <= 5)
					{
						brightnessIndex = 1;
					}
					else if (brightness <= 10)
					{
						brightnessIndex = 2;
					}
					else if (brightness <= 15)
					{
						brightnessIndex = 3;
					}
					else if (brightness <= 20)
					{
						brightnessIndex = 4;
					}
					else if (brightness <= 25)
					{
						brightnessIndex = 5;
					}
					else if (brightness <= 30)
					{
						brightnessIndex = 6;
					}
					else if (brightness <= 35)
					{
						brightnessIndex = 7;
					}
					else if (brightness <= 40)
					{
						brightnessIndex = 8;
					}
					else if (brightness <= 45)
					{
						brightnessIndex = 9;
					}
					else if (brightness <= 50)
					{
						brightnessIndex = 10;
					}
					else if (brightness <= 55)
					{
						brightnessIndex = 11;
					}
					else if (brightness <= 60)
					{
						brightnessIndex = 12;
					}
					else if (brightness <= 65)
					{
						brightnessIndex = 13;
					}
					else if (brightness <= 70)
					{
						brightnessIndex = 14;
					}
					else if (brightness <= 75)
					{
						brightnessIndex = 15;
					}
					else if (brightness <= 80)
					{
						brightnessIndex = 16;
					}
					else if (brightness <= 85)
					{
						brightnessIndex = 17;
					}
					else if (brightness <= 90)
					{
						brightnessIndex = 18;
					}
					else if (brightness <= 95)
					{
						brightnessIndex = 19;
					}
					else if (brightness = 100)
					{
						brightnessIndex = 20;
					}
					else
					{
						brightnessIndex = 20;
					}

					src += (brightnessIndex * 16 * src_stride);
					dst += (64) * sizeof(short);

					for (int y = 0; y < 16; ++y)
					{
						memcpy(dst, src, 32 * sizeof(short));

						src += src_stride;
						dst += dst_stride;
					}
				} // Brightness level END

				{

					// WIFI ICONS
					const uint8_t* src = wifi_image.pixel_data;
					int src_stride = 32 * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					int wifiIndex = 0;

					// Wifi
					int fd;
					char buffer[10];
					fd = open("/sys/class/net/wlan0/operstate", O_RDONLY);
					if (fd > 0)
					{
						memset(buffer, 0, 10);
						ssize_t count = read(fd, buffer, 10);
						if( count > 0 )
						{
							if( strstr( buffer, "up") != NULL )
								wifiIndex = 1;
							else
								wifiIndex = 0;
						}
						close(fd);
					}
					if( wifiIndex == 1 )
					{
						src += (wifiIndex* 16 * src_stride);
						dst += (w - 100) * sizeof(short);

						for (int y = 0; y < 16; ++y)
						{
							memcpy(dst, src, 32 * sizeof(short));

							src += src_stride;
							dst += dst_stride;
						}

					}
					else
					{
						const uint8_t* src = blank_image.pixel_data;
						src += (1 * 16 * src_stride);
						dst += (w - 100) * sizeof(short);

						for (int y = 0; y < 16; ++y)
						{
							memcpy(dst, src, 32 * sizeof(short));

							src += src_stride;
							dst += dst_stride;
						}

					}
				}// WIFI ICONS END

				{

					// POWER ICON
					const uint8_t* src = power_image.pixel_data;
					int src_stride = 32 * sizeof(short);

					uint8_t* dst = (uint8_t*)go2_surface_map(titlebarSurface);
					int dst_stride = go2_surface_stride_get(titlebarSurface);

					int powerIndex = 0;

					// Power
					int fd;
					char buffer[10];
					fd = open("/sys/class/power_supply/ac/online", O_RDONLY);
					if (fd > 0)
					{
						memset(buffer, 0, 10);
						ssize_t count = read(fd, buffer, 10);
						if( count > 0 )
						{
							if( strstr( buffer, "1") != 0 )
								powerIndex = 1;
							else
								powerIndex = 0;
						}
						close(fd);
					}
					if( powerIndex == 1 )
					{
						src += (powerIndex * 16 * src_stride);
						dst += (w - 65) * sizeof(short);

						for (int y = 0; y < 16; ++y)
						{
							memcpy(dst, src, 32 * sizeof(short));

							src += src_stride;
							dst += dst_stride;
						}

					}
					else
					{
						const uint8_t* src = blank_image.pixel_data;
						src += (1 * 16 * src_stride);
						dst += (w - 65) * sizeof(short);

						for (int y = 0; y < 16; ++y)
						{
							memcpy(dst, src, 32 * sizeof(short));

							src += src_stride;
							dst += dst_stride;
						}

					}
				} // POWER ICON - END

			} // if (!Renderer::isFullScreenMode())

			go2_context_swap_buffers(context);
			go2_surface_t* surface = go2_context_surface_lock(context);

			if (!Renderer::isFullScreenMode())
				go2_surface_blit(titlebarSurface, 0, 0, w, 16,
								 surface, 0, 0, w, 16,
								 GO2_ROTATION_DEGREES_0);

			go2_presenter_post(presenter,
						surface,
						0, 0, w, h,
						0, 0, h, w,
						GO2_ROTATION_DEGREES_270);
			go2_context_surface_unlock(context, surface);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//printf("Frame %d\n", frame++);
	} // swapBuffers

#define ROUNDING_PIECES 8.0f

	void drawGLRoundedCorner(float x, float y, double sa, double arc, float r, unsigned int color, std::vector<Vertex> &vertex)
	{
		float red = (((color & 0xff000000) >> 24) & 255) / 255.0f;
		float g = (((color & 0x00ff0000) >> 16) & 255) / 255.0f;
		float b = (((color & 0x0000ff00) >> 8) & 255) / 255.0f;
		float a = (((color & 0x000000ff)) & 255) / 255.0f;

		// centre of the arc, for clockwise sense
		float cent_x = x + r * Math::cosf(sa + ES_PI / 2.0f);
		float cent_y = y + r * Math::sinf(sa + ES_PI / 2.0f);

		// build up piecemeal including end of the arc
		int n = ceil(ROUNDING_PIECES * arc / ES_PI * 2.0f);
		for (int i = 0; i <= n; i++)
		{
			float ang = sa + arc * (double)i / (double)n;

			// compute the next point
			float next_x = cent_x + r * Math::sinf(ang);
			float next_y = cent_y - r * Math::cosf(ang);

			Vertex vx;
			vx.pos = Vector2f(next_x, next_y);
			vx.tex = Vector2f(0, 0);
			vx.col = color;
			vertex.push_back(vx);
		}
	}

	void drawRoundRect(float x, float y, float width, float height, float radius, unsigned int color, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		auto finalColor = convertColor(color);

		std::vector<Vertex> vertex;
		drawGLRoundedCorner(x, y + radius, 3.0f * ES_PI / 2.0f, ES_PI / 2.0f, radius, finalColor, vertex);
		drawGLRoundedCorner(x + width - radius, y, 0.0, ES_PI / 2.0f, radius, finalColor, vertex);
		drawGLRoundedCorner(x + width, y + height - radius, ES_PI / 2.0f, ES_PI / 2.0f, radius, finalColor, vertex);
		drawGLRoundedCorner(x + radius, y + height, ES_PI, ES_PI / 2.0f, radius, finalColor, vertex);

		Vertex* vxs = new Vertex[vertex.size()];
		for (int i = 0; i < vertex.size(); i++)
			vxs[i] = vertex[i];

		bindTexture(0);

		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vxs[0].pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vxs[0].tex);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vxs[0].col);

		glDrawArrays(GL_TRIANGLE_FAN, 0, vertex.size());

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_BLEND);

		delete[] vxs;	
	}

	void enableRoundCornerStencil(float x, float y, float width, float height, float radius)
	{
		bool tx = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);

		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);

		drawRoundRect(x, y, width, height, radius, 0xFFFFFFFF);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		glStencilFunc(GL_EQUAL, 1, 0xFF);

		if (tx)
			glEnable(GL_TEXTURE_2D);
	}

	void disableStencil()
	{
		glDisable(GL_STENCIL_TEST);
	}
} // Renderer::

#endif // USE_OPENGLES_10
