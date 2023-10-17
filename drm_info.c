#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

static struct {
    int fd;
    drmModeModeInfo *mode;
    uint32_t crtc_id;
    uint32_t connector_id;
} drm;

char *encoder_str[] = {
"DRM_MODE_ENCODER_NONE",
"DRM_MODE_ENCODER_DAC",
"DRM_MODE_ENCODER_TMDS",
"DRM_MODE_ENCODER_LVDS",
"DRM_MODE_ENCODER_TVDAC",
"DRM_MODE_ENCODER_VIRTUAL",
"DRM_MODE_ENCODER_DSI",
"DRM_MODE_ENCODER_DPMST",
"DRM_MODE_ENCODER_DPI"
};


int main()
{
    drmModeRes *resources;
    drmModeCrtc *crtc = NULL;
    drmModeConnector *connector = NULL;
    drmModeEncoder *encoder = NULL;
    int i, area;

    for (int card =0;card<4;card++)
    {
        char filename[20];
        sprintf(filename, "/dev/dri/card%d", card);

        drm.fd = open(filename, O_RDWR);
        
        if (drm.fd < 0) {
            printf("could not open drm device %s\n", filename);
            return -1;
        }

        resources = drmModeGetResources(drm.fd);
        if (!resources)
        {
            printf("Could not get resources from %s, continuing\n", filename);
            continue;
        }
        
        break;
    }

    drmVersionPtr ver = drmGetVersion(drm.fd);

    printf("Version: %d.%d, '%.*s', '%.*s', '%.*s'\n",  ver->version_major, ver->version_minor, ver->name_len, ver->name, ver->date_len, ver->date, ver->desc_len, ver->desc);
    
    printf("Resources:\n");

    for (i = 0; i < resources->count_crtcs; i++)
    {
        if (i == 0)
            printf("├ CRTCs (%d):\n", resources->count_crtcs);

        crtc = drmModeGetCrtc(drm.fd, resources->crtcs[i]);

        if (i == resources->count_crtcs-1)
            printf("│  └─");
        else    
            printf("│  ├─");

        printf(" CRTC ID: %d, X pos %d, Y pos %d, width %d, height %d\n", crtc->crtc_id, crtc->x, crtc->y, crtc->width, crtc->height);

        drmModeObjectProperties *props = drmModeObjectGetProperties(drm.fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);

        if (props->count_props != 0)
            if (props->count_props > 1)
                if (i == resources->count_crtcs - 1)
                    printf("│     ├─ Properties (%d):\n", props->count_props);
                else
                    printf("│  │  ├─ Properties (%d):\n", props->count_props);
            else
                if (i == resources->count_crtcs - 1)
                    printf("│     └─ Properties (%d):\n", props->count_props);
                else
                    printf("│  │  └─ Properties (%d):\n", props->count_props);


        for (int p = 0;p < props->count_props; p++)
        {
            drmModePropertyRes *prop = drmModeGetProperty(drm.fd, props->props[p]);
            if (p == props->count_props-1)
                if (i == resources->count_crtcs - 1)
                    printf("│        └─");
                else
                    printf("│  │     └─");
            else    
                if (i == resources->count_crtcs - 1)
                    printf("│        ├─");
                else
                    printf("│  │     ├─");

            printf(" ID : %d, Name : '%s'\n", prop->prop_id, prop->name);

        }


        drmModeFreeCrtc(crtc);
    }

    for (i = 0; i < resources->count_connectors; i++)
    {
        if (i == 0)
            printf("├─ Connectors (%d):\n", resources->count_connectors);

        connector = drmModeGetConnector(drm.fd, resources->connectors[i]);

        printf("│  ├─ Connector ID: %d\n", connector->connector_id);
        printf("│  ├─ Encoder ID: %d\n", connector->encoder_id);
        printf("│  ├─ Connector type  ID: %d\n", connector->connector_type_id);
        printf("│  ├─ Connected? : %s\n", connector->connection == DRM_MODE_CONNECTED ? "yes" : "No");

        if (i == resources->count_connectors-1)
            printf("│  └─ ");
        else    
            printf("│  ├─ ");
        
        printf("Modes (%d):\n", connector->count_modes);
        
        for (int m = 0, area = 0; m < connector->count_modes; m++)
        {
            drmModeModeInfo *current_mode = &connector->modes[m];
            if (m == connector->count_modes-1)
                if (i == resources->count_connectors-1)
                    printf("│     └─");
                else    
                    printf("│  │  └─");
            else
                if (i == resources->count_connectors-1)
                    printf("│     ├─");
                else    
                    printf("│  │  ├─");

            printf(" Name: '%s', width %d, height %d, refresh %d\n", current_mode->name, current_mode->hdisplay, current_mode->vdisplay, current_mode->vrefresh);
        }

        drmModeFreeConnector(connector);
    }

    /* find encoder: */
    for (i = 0; i < resources->count_encoders; i++) {
        bool current;

        if (i == 0)
            printf("└ Encoders (%d):\n", resources->count_encoders);

        encoder = drmModeGetEncoder(drm.fd, resources->encoders[i]);
        if (encoder->encoder_id == connector->encoder_id)
            current = true;
        else 
            current = false;

        if ( i == resources->count_encoders -1)
            printf("   └─");
        else
            printf("   ├─");

        printf(" Encoder ID %d, Encoder Type %d (%s), CRTC id %d", encoder->encoder_id, encoder->encoder_type, encoder_str[encoder->encoder_type], encoder->crtc_id);

        if (encoder->encoder_id == connector->encoder_id)
            printf("(Current)\n");
        else 
            printf("\n");

        drmModeFreeEncoder(encoder);
        encoder = NULL;
    }

    return 0;
}
