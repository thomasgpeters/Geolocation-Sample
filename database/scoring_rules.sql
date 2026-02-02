-- ============================================================================
-- Scoring Rules Table
-- Stores configurable scoring rules for prospect evaluation
-- ============================================================================

CREATE TABLE scoring_rules (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    rule_id             VARCHAR(50) NOT NULL,           -- Unique rule identifier (e.g., 'no_address')
    name                VARCHAR(100) NOT NULL,          -- Display name
    description         TEXT,                           -- Rule description
    is_penalty          BOOLEAN NOT NULL DEFAULT FALSE, -- True for penalties, false for bonuses
    enabled             BOOLEAN NOT NULL DEFAULT TRUE,  -- Whether rule is active
    default_points      INTEGER NOT NULL DEFAULT 0,     -- Default point adjustment
    current_points      INTEGER NOT NULL DEFAULT 0,     -- Current configured adjustment
    min_points          INTEGER NOT NULL DEFAULT -50,   -- Minimum allowed value
    max_points          INTEGER NOT NULL DEFAULT 50,    -- Maximum allowed value
    franchisee_id       UUID,                           -- NULL = global rule, otherwise franchisee-specific
    created_at          TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at          TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,

    -- Foreign key to franchisee table (optional - NULL means global rule)
    CONSTRAINT fk_scoring_rules_franchisee
        FOREIGN KEY (franchisee_id)
        REFERENCES franchisee(id)
        ON DELETE CASCADE,

    -- Ensure rule_id is unique per franchisee (or globally if franchisee_id is NULL)
    CONSTRAINT uq_scoring_rules_rule_franchisee
        UNIQUE (rule_id, franchisee_id)
);

-- ============================================================================
-- Indexes
-- ============================================================================

-- Index for looking up rules by franchisee
CREATE INDEX idx_scoring_rules_franchisee_id
    ON scoring_rules(franchisee_id);

-- Index for finding enabled rules quickly
CREATE INDEX idx_scoring_rules_enabled
    ON scoring_rules(enabled)
    WHERE enabled = TRUE;

-- Index for rule lookups by rule_id
CREATE INDEX idx_scoring_rules_rule_id
    ON scoring_rules(rule_id);

-- Composite index for common query pattern: active rules for a franchisee
CREATE INDEX idx_scoring_rules_franchisee_enabled
    ON scoring_rules(franchisee_id, enabled);

-- ============================================================================
-- Trigger for updated_at timestamp
-- ============================================================================

CREATE OR REPLACE FUNCTION update_scoring_rules_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_scoring_rules_updated_at
    BEFORE UPDATE ON scoring_rules
    FOR EACH ROW
    EXECUTE FUNCTION update_scoring_rules_updated_at();

-- ============================================================================
-- Sample Data - Global Default Rules (franchisee_id = NULL)
-- ============================================================================

INSERT INTO scoring_rules (id, rule_id, name, description, is_penalty, enabled, default_points, current_points, min_points, max_points, franchisee_id) VALUES

-- Penalty Rules (subtract points for negative indicators)
('11111111-1111-1111-1111-111111111001', 'no_address', 'Missing Address',
 'Business has no street address listed', TRUE, TRUE, -15, -15, -50, 0, NULL),

('11111111-1111-1111-1111-111111111002', 'no_phone', 'Missing Phone Number',
 'Business has no phone number listed', TRUE, TRUE, -10, -10, -50, 0, NULL),

('11111111-1111-1111-1111-111111111003', 'no_website', 'Missing Website',
 'Business has no website listed', TRUE, TRUE, -5, -5, -50, 0, NULL),

('11111111-1111-1111-1111-111111111004', 'closed_business', 'Potentially Closed',
 'Business shows signs of being closed or inactive', TRUE, TRUE, -25, -25, -50, 0, NULL),

('11111111-1111-1111-1111-111111111005', 'incomplete_hours', 'Incomplete Hours',
 'Business hours are not fully listed', TRUE, TRUE, -5, -5, -50, 0, NULL),

('11111111-1111-1111-1111-111111111006', 'low_reviews', 'Few Reviews',
 'Business has very few customer reviews', TRUE, TRUE, -8, -8, -50, 0, NULL),

('11111111-1111-1111-1111-111111111007', 'poor_rating', 'Poor Rating',
 'Business has below average customer rating', TRUE, TRUE, -12, -12, -50, 0, NULL),

-- Bonus Rules (add points for positive indicators)
('11111111-1111-1111-1111-111111111101', 'verified_business', 'Verified Business',
 'Business ownership has been verified', FALSE, TRUE, 10, 10, 0, 50, NULL),

('11111111-1111-1111-1111-111111111102', 'complete_profile', 'Complete Profile',
 'Business has all contact information listed', FALSE, TRUE, 8, 8, 0, 50, NULL),

('11111111-1111-1111-1111-111111111103', 'high_reviews', 'Many Reviews',
 'Business has substantial number of customer reviews', FALSE, TRUE, 12, 12, 0, 50, NULL),

('11111111-1111-1111-1111-111111111104', 'excellent_rating', 'Excellent Rating',
 'Business has above average customer rating', FALSE, TRUE, 15, 15, 0, 50, NULL),

('11111111-1111-1111-1111-111111111105', 'established_business', 'Established Business',
 'Business has been operating for multiple years', FALSE, TRUE, 10, 10, 0, 50, NULL),

('11111111-1111-1111-1111-111111111106', 'active_online', 'Active Online Presence',
 'Business actively maintains their online profiles', FALSE, TRUE, 7, 7, 0, 50, NULL),

('11111111-1111-1111-1111-111111111107', 'ideal_size', 'Ideal Employee Count',
 'Business size matches target employee range', FALSE, TRUE, 10, 10, 0, 50, NULL),

('11111111-1111-1111-1111-111111111108', 'target_industry', 'Target Industry',
 'Business operates in a preferred industry category', FALSE, TRUE, 15, 15, 0, 50, NULL);

-- ============================================================================
-- Verify data
-- ============================================================================

SELECT COUNT(*) as total_rules,
       SUM(CASE WHEN is_penalty THEN 1 ELSE 0 END) as penalties,
       SUM(CASE WHEN NOT is_penalty THEN 1 ELSE 0 END) as bonuses
FROM scoring_rules;
