-- =====================================================
-- ScoringRule Table for ApiLogicServer
-- Stores configurable scoring rules for prospect evaluation
-- =====================================================

-- Drop table if exists (for development/migration)
-- DROP TABLE IF EXISTS scoring_rule;

CREATE TABLE scoring_rule (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Rule identification
    rule_id         VARCHAR(50) NOT NULL UNIQUE,    -- e.g., 'no_address', 'bbb_accredited'
    name            VARCHAR(100) NOT NULL,           -- Display name: 'Missing Address'
    description     VARCHAR(500),                    -- Explanation of rule

    -- Rule configuration
    is_penalty      BOOLEAN DEFAULT FALSE,           -- True for penalties, False for bonuses
    enabled         BOOLEAN DEFAULT TRUE,            -- Whether rule is active
    default_points  INTEGER NOT NULL,                -- Default point adjustment
    current_points  INTEGER NOT NULL,                -- Current configured adjustment
    min_points      INTEGER DEFAULT -50,             -- Minimum allowed value
    max_points      INTEGER DEFAULT 50,              -- Maximum allowed value

    -- Ownership (for multi-tenant support)
    franchisee_id   INTEGER,                         -- Optional: rule belongs to specific franchisee

    -- Timestamps
    created_at      DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at      DATETIME DEFAULT CURRENT_TIMESTAMP,

    -- Foreign key (optional, uncomment if franchisee table exists)
    -- FOREIGN KEY (franchisee_id) REFERENCES franchisee(id)

    -- Indexes for common queries
    CONSTRAINT chk_points_range CHECK (current_points >= min_points AND current_points <= max_points)
);

-- Index for fast lookups
CREATE INDEX idx_scoring_rule_rule_id ON scoring_rule(rule_id);
CREATE INDEX idx_scoring_rule_franchisee ON scoring_rule(franchisee_id);
CREATE INDEX idx_scoring_rule_enabled ON scoring_rule(enabled);

-- =====================================================
-- Default scoring rules (insert on first run)
-- =====================================================

-- Penalty rules (negative adjustments)
INSERT INTO scoring_rule (rule_id, name, description, is_penalty, enabled, default_points, current_points, min_points, max_points)
VALUES
    ('no_address', 'Missing Address', 'Prospects without addresses are harder to contact and verify', TRUE, TRUE, -10, -10, -25, 0),
    ('no_employees', 'Missing Employee Count', 'Unknown employee count makes catering potential harder to estimate', TRUE, TRUE, -3, -3, -15, 0),
    ('no_contact', 'Missing Contact Info', 'No phone or email makes outreach difficult', TRUE, TRUE, -5, -5, -20, 0);

-- Bonus rules (positive adjustments)
INSERT INTO scoring_rule (rule_id, name, description, is_penalty, enabled, default_points, current_points, min_points, max_points)
VALUES
    ('verified', 'Verified Business', 'Business has been verified through data sources', FALSE, TRUE, 5, 5, 0, 15),
    ('bbb_accredited', 'BBB Accredited', 'Business is accredited by the Better Business Bureau', FALSE, TRUE, 10, 10, 0, 20),
    ('high_rating', 'High Google Rating', 'Business has 4.5+ star Google rating', FALSE, TRUE, 5, 5, 0, 15),
    ('conference_room', 'Has Conference Room', 'Business has conference facilities - good for catering', FALSE, TRUE, 5, 5, 0, 15),
    ('event_space', 'Has Event Space', 'Business has dedicated event space', FALSE, TRUE, 7, 7, 0, 20),
    ('large_company', 'Large Company (100+ employees)', 'Larger companies have more catering opportunities', FALSE, TRUE, 8, 8, 0, 20);

-- =====================================================
-- Trigger to update updated_at timestamp
-- =====================================================

CREATE TRIGGER update_scoring_rule_timestamp
AFTER UPDATE ON scoring_rule
BEGIN
    UPDATE scoring_rule SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- =====================================================
-- View for easy querying of active rules
-- =====================================================

CREATE VIEW active_scoring_rules AS
SELECT
    rule_id,
    name,
    description,
    is_penalty,
    current_points,
    min_points,
    max_points
FROM scoring_rule
WHERE enabled = TRUE
ORDER BY is_penalty DESC, ABS(current_points) DESC;

-- =====================================================
-- Example queries
-- =====================================================

-- Get all rules for display in Settings UI:
-- SELECT * FROM scoring_rule ORDER BY is_penalty DESC, name;

-- Get only enabled rules for scoring:
-- SELECT rule_id, current_points, is_penalty FROM scoring_rule WHERE enabled = TRUE;

-- Update a rule's points:
-- UPDATE scoring_rule SET current_points = -15 WHERE rule_id = 'no_address';

-- Toggle a rule:
-- UPDATE scoring_rule SET enabled = FALSE WHERE rule_id = 'no_contact';

-- Reset all rules to defaults:
-- UPDATE scoring_rule SET current_points = default_points, enabled = TRUE;

-- Get rules for a specific franchisee (if multi-tenant):
-- SELECT * FROM scoring_rule WHERE franchisee_id = ? OR franchisee_id IS NULL;
