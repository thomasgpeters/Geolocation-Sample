# FranchiseAI Documentation

This folder contains all technical and user documentation for the FranchiseAI Prospect Search Application.

## Document Index

### Core Documentation

| Document | Description |
|----------|-------------|
| [RELEASE_NOTES_v2.md](RELEASE_NOTES_v2.md) | **NEW** - Version 2.0 release notes covering Hot Prospects, Scoring Engine, and performance improvements |
| [USER_GUIDE.md](USER_GUIDE.md) | End-user guide for franchise owners - how to use the application to find catering prospects |
| [FUNCTIONAL_DOC.md](FUNCTIONAL_DOC.md) | Functional specification detailing all features, workflows, and business logic |
| [UI_GUIDE.md](UI_GUIDE.md) | UI/UX design guide with component specifications, styling guidelines, and layout patterns |

### Technical Documentation

| Document | Description |
|----------|-------------|
| [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md) | Development roadmap for each interface - current state, planned enhancements, and priorities |
| [PERFORMANCE_OPTIMIZATION.md](PERFORMANCE_OPTIMIZATION.md) | Performance tuning guide covering API optimization, caching, progressive loading, and scoring engine |
| [GEOLOCATION_OPTIONS.md](GEOLOCATION_OPTIONS.md) | Analysis of geolocation service options - Google, OpenStreetMap, Nominatim, and alternatives |
| [SPRINT_AUTH_PLAN.md](SPRINT_AUTH_PLAN.md) | Authentication sprint plan - JWT implementation, session management, and security features |

### Business Documentation

| Document | Description |
|----------|-------------|
| [PROPOSAL_DECK.md](PROPOSAL_DECK.md) | Product proposal deck in markdown format - features, benefits, and market positioning |
| [PROPOSAL_DECK.pdf](PROPOSAL_DECK.pdf) | PDF version of the proposal deck (generated) |
| [PROPOSAL_DECK.pptx](PROPOSAL_DECK.pptx) | PowerPoint version of the proposal deck (generated) |
| [PROPOSAL_DECK.html](PROPOSAL_DECK.html) | HTML version of the proposal deck for web presentations |

## Generating Presentation Slides

The proposal deck uses [Marp](https://marp.app/) for converting markdown to PDF, PowerPoint, and HTML formats.

### Prerequisites

- Node.js 16+ installed ([download](https://nodejs.org/))

### Quick Start

```bash
# Navigate to docs folder
cd docs

# Generate all formats (PDF, PPTX, HTML)
./generate-slides.sh

# Or generate specific format
./generate-slides.sh pdf    # PDF only
./generate-slides.sh pptx   # PowerPoint only
./generate-slides.sh html   # HTML only

# Live preview with hot reload
./generate-slides.sh watch
```

### Using npm directly

```bash
cd docs
npm install                  # First time only
npm run slides:pdf           # Generate PDF
npm run slides:pptx          # Generate PowerPoint
npm run slides:html          # Generate HTML
npm run slides:all           # Generate all formats
npm run slides:watch         # Live preview server
```

### Output Files

| File | Format | Use Case |
|------|--------|----------|
| `PROPOSAL_DECK.pdf` | PDF | Print, email attachments, formal presentations |
| `PROPOSAL_DECK.pptx` | PowerPoint | Editable slides, Microsoft Office users |
| `PROPOSAL_DECK.html` | HTML | Web viewing, screen sharing |

### Customizing Slides

The presentation styling is configured in the YAML frontmatter at the top of `PROPOSAL_DECK.md`:

```yaml
---
marp: true
theme: default
paginate: true
backgroundColor: #fff
style: |
  h1 { color: #2563eb; }
  h2 { color: #1e40af; }
---
```

For more customization options, see the [Marp documentation](https://marpit.marp.app/directives).

### Architecture Diagrams

| File | Description |
|------|-------------|
| [infrastructure-diagram.svg](infrastructure-diagram.svg) | System infrastructure diagram showing service components and data flow |
| [mcp-architecture-diagram.svg](mcp-architecture-diagram.svg) | MCP (Model Context Protocol) architecture for AI integration |

## Quick Links

- **What's New**: See [RELEASE_NOTES_v2.md](RELEASE_NOTES_v2.md) for v2.0 features and improvements
- **Getting Started**: See [USER_GUIDE.md](USER_GUIDE.md) for application usage
- **API Performance**: See [PERFORMANCE_OPTIMIZATION.md](PERFORMANCE_OPTIMIZATION.md) for optimization details
- **Development**: See [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md) for roadmap and priorities
- **Database Setup**: See [../database/README.md](../database/README.md) for ApiLogicServer setup

## Related Files

- Main project README: [../README.md](../README.md)
- Database schema: [../database/schema.sql](../database/schema.sql)
- Configuration: [../config/](../config/)
