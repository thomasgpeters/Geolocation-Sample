#!/bin/bash
# ============================================================================
# FranchiseAI Slide Generation Script
# Converts PROPOSAL_DECK.md to PDF, PPTX, and HTML formats
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}FranchiseAI Slide Generator${NC}"
echo "========================================"

# Check if npm/npx is available
if ! command -v npx &> /dev/null; then
    echo -e "${RED}Error: npx is not installed. Please install Node.js first.${NC}"
    echo "Visit: https://nodejs.org/"
    exit 1
fi

# Check if node_modules exists, if not install dependencies
if [ ! -d "node_modules" ]; then
    echo -e "${YELLOW}Installing dependencies...${NC}"
    npm install
fi

# Parse command line arguments
FORMAT="${1:-all}"

case "$FORMAT" in
    pdf)
        echo -e "${YELLOW}Generating PDF...${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --pdf --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.pdf${NC}"
        ;;
    pptx)
        echo -e "${YELLOW}Generating PowerPoint...${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --pptx --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.pptx${NC}"
        ;;
    html)
        echo -e "${YELLOW}Generating HTML...${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --html --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.html${NC}"
        ;;
    watch)
        echo -e "${YELLOW}Starting live preview server...${NC}"
        echo "Open http://localhost:8080 in your browser"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --watch --server
        ;;
    all)
        echo -e "${YELLOW}Generating all formats...${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --pdf --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.pdf${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --pptx --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.pptx${NC}"
        npx @marp-team/marp-cli PROPOSAL_DECK.md --html --allow-local-files
        echo -e "${GREEN}Created: PROPOSAL_DECK.html${NC}"
        ;;
    *)
        echo "Usage: ./generate-slides.sh [format]"
        echo ""
        echo "Formats:"
        echo "  pdf    - Generate PDF slides"
        echo "  pptx   - Generate PowerPoint presentation"
        echo "  html   - Generate HTML slides"
        echo "  watch  - Start live preview server"
        echo "  all    - Generate PDF, PPTX, and HTML (default)"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}Done!${NC}"
